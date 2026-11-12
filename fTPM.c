/* Microsoft Reference Implementation for TPM 2.0
 *
 *  The copyright in this software is being made available under the BSD License,
 *  included below. This software may be subject to other third party and
 *  contributor rights, including patent rights, and no such rights are granted
 *  under this license.
 *
 *  Copyright (c) Microsoft Corporation
 *  Copyright (c) Arm Limited.
 *
 *  All rights reserved.
 *
 *  BSD License
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright notice, this list
 *  of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright notice, this
 *  list of conditions and the following disclaimer in the documentation and/or
 *  other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS""
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define STR_TRACE_USER_TA "fTPM"

#include <config.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>
#include <pta_system.h>
#include <fTPM_helpers.h>
#include <fTPM_event_log.h>

#include "fTPM.h"

//
// Ensure we have only one active session
//
static bool fTPMSessionActive = false;

//
// Initialization
//
bool fTPMInitialized = false;

//
// Track RPMB storage availability for graceful degradation
//
static bool fTPM_storage_available = true;

//
// Local (SW) command buffer
//
static uint8_t fTPMCommand[MAX_COMMAND_SIZE];

//
// A subset of TPM return codes (see TpmTypes.h)
//
typedef uint32_t TPM_RC;
#define RC_VER1             (TPM_RC) (0x100)
#define TPM_RC_SUCCESS      (TPM_RC) (0x000)
#define TPM_RC_FAILURE      (TPM_RC) (RC_VER1+0x001)

//
// Helper function to read response codes from TPM responses
//
static uint32_t fTPMResponseCode(uint32_t ResponseSize,
                                 uint8_t *ResponseBuffer)
{
    uint32_t ResponseCode;
    union {
        uint32_t Data;
        uint8_t Index[4];
    } Value;

    // In case of too-small response size, assume failure.
    if (ResponseSize < 0xA) {
        return TPM_RC_FAILURE;
    }

    Value.Index[0] = ResponseBuffer[6];
    Value.Index[1] = ResponseBuffer[7];
    Value.Index[2] = ResponseBuffer[8];
    Value.Index[3] = ResponseBuffer[9];
    ResponseCode = SwapBytes32(Value.Data);

    return ResponseCode;
}

#ifdef MEASURED_BOOT
static TEE_Result get_tpm_event_log(unsigned char *buf, size_t *len)
{
    const TEE_UUID system_uuid = PTA_SYSTEM_UUID;
    TEE_TASessionHandle session = TEE_HANDLE_NULL;
    TEE_Result res = TEE_ERROR_GENERIC;
    uint32_t ret_origin = 0;
    const uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                                 TEE_PARAM_TYPE_NONE,
                                                 TEE_PARAM_TYPE_NONE,
                                                 TEE_PARAM_TYPE_NONE);
    TEE_Param params[TEE_NUM_PARAMS] = {0};

    res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
                            0, NULL, &session, &ret_origin);
    if (res != TEE_SUCCESS)
        return res;

    params[0].memref.buffer = (void *)buf;
    params[0].memref.size = *len;

    res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
                              PTA_SYSTEM_GET_TPM_EVENT_LOG,
                              param_types, params, &ret_origin);

    *len = params[0].memref.size;

    TEE_CloseTASession(session);

    return res;
}
#endif // MEASURED_BOOT

//
// Early mode: TPM use before secure storage is reachable.
//
// The fTPM keeps its NV state in OP-TEE secure storage. With REE FS that
// storage is served by tee-supplicant, which does not exist before the OS
// runs, so a boot loader (e.g. U-Boot's tpm2_ftpm_tee driver measuring the
// kernel) cannot use the fTPM at all: every session used to be refused.
//
// Instead, when storage does not answer, the fTPM now starts on a temporary,
// volatile TPM state (manufactured in memory, never written back) and serves
// only commands that need no NV: PCR extend/read, capabilities, random
// numbers, self test. Every successful PCR_Extend/PCR_Event is recorded in a
// journal. At every session open and command the fTPM checks whether storage
// answers now; once it does, the volatile state is dropped, the real NV state
// is loaded, the TPM is started (exactly as at a normal first open) and the
// journal is replayed, so the PCRs hold the measurements taken before
// storage was available, followed by everything after. Commands that need NV
// get TPM_RC_NV_UNAVAILABLE while in early mode.
//
#define TPM_ST_NO_SESSIONS          0x8001
#define TPM_ST_SESSIONS             0x8002
#define TPM_RS_PW                   0x40000009
#define TPM_RC_MEMORY_W             ((TPM_RC)0x904)
#define TPM_RC_NV_UNAVAILABLE_W     ((TPM_RC)0x923)

#define TPM_CC_IncrementalSelfTest  0x00000142
#define TPM_CC_SelfTest             0x00000143
#define TPM_CC_Startup              0x00000144
#define TPM_CC_PCR_Event            0x0000013C
#define TPM_CC_GetCapability        0x0000017A
#define TPM_CC_GetRandom            0x0000017B
#define TPM_CC_GetTestResult        0x0000017C
#define TPM_CC_Hash                 0x0000017D
#define TPM_CC_PCR_Read             0x0000017E
#define TPM_CC_ReadClock            0x00000181
#define TPM_CC_PCR_Extend           0x00000182

static bool fTPM_early = false;
static uint8_t fTPM_journal[CFG_FTPM_EARLY_JOURNAL_SIZE];
static uint32_t fTPM_journal_len = 0;
static uint8_t fTPM_replay_response[MAX_RESPONSE_SIZE];

static uint32_t fTPM_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t fTPM_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static bool fTPM_early_allowed(uint32_t cc)
{
    switch (cc) {
    case TPM_CC_IncrementalSelfTest:
    case TPM_CC_SelfTest:
    case TPM_CC_Startup:
    case TPM_CC_PCR_Event:
    case TPM_CC_GetCapability:
    case TPM_CC_GetRandom:
    case TPM_CC_GetTestResult:
    case TPM_CC_Hash:
    case TPM_CC_PCR_Read:
    case TPM_CC_ReadClock:
    case TPM_CC_PCR_Extend:
        return true;
    default:
        return false;
    }
}

//
// A PCR_Extend/PCR_Event can only be replayed later if it carries exactly one
// plain password session (the PCR auth value cannot be changed in early mode).
//
static bool fTPM_journal_ok(const uint8_t *cmd, uint32_t len)
{
    uint32_t authSize;

    if (len < 27 || fTPM_be16(cmd) != TPM_ST_SESSIONS)
        return false;
    authSize = fTPM_be32(cmd + 14);
    return fTPM_be32(cmd + 18) == TPM_RS_PW &&
           fTPM_be16(cmd + 22) == 0 &&
           authSize == 9 + (uint32_t)fTPM_be16(cmd + 25) &&
           18 + authSize <= len;
}

static void fTPM_rc_response(TPM_RC rc, uint32_t *respLen, uint8_t *respBuf)
{
    static const uint8_t hdr[6] = { 0x80, 0x01, 0x00, 0x00, 0x00, 0x0A };

    memcpy(respBuf, hdr, sizeof(hdr));
    respBuf[6] = (uint8_t)(rc >> 24);
    respBuf[7] = (uint8_t)(rc >> 16);
    respBuf[8] = (uint8_t)(rc >> 8);
    respBuf[9] = (uint8_t)rc;
    *respLen = 10;
}

//
// Execute one command in early mode; respBuf/respLen as for ExecuteCommand().
//
static void fTPM_early_execute(uint32_t cmdLen, uint8_t *cmdBuf,
                               uint32_t *respLen, uint8_t **respBuf)
{
    uint32_t cc = fTPM_be32(cmdBuf + 6);
    bool journal = (cc == TPM_CC_PCR_Extend || cc == TPM_CC_PCR_Event);

    if (*respLen < 10) {
        *respLen = 10;
        return;
    }
    if (!fTPM_early_allowed(cc) ||
        (journal && !fTPM_journal_ok(cmdBuf, cmdLen))) {
        fTPM_rc_response(TPM_RC_NV_UNAVAILABLE_W, respLen, *respBuf);
        return;
    }
    if (journal && fTPM_journal_len + 4 + cmdLen > sizeof(fTPM_journal)) {
        EMSG("fTPM: early mode journal full, PCR extend refused");
        fTPM_rc_response(TPM_RC_MEMORY_W, respLen, *respBuf);
        return;
    }

    ExecuteCommand(cmdLen, cmdBuf, respLen, respBuf);

    if (journal && fTPMResponseCode(*respLen, *respBuf) == TPM_RC_SUCCESS) {
        uint8_t *p = fTPM_journal + fTPM_journal_len;

        p[0] = (uint8_t)(cmdLen >> 24);
        p[1] = (uint8_t)(cmdLen >> 16);
        p[2] = (uint8_t)(cmdLen >> 8);
        p[3] = (uint8_t)cmdLen;
        memcpy(p + 4, cmdBuf, cmdLen);
        fTPM_journal_len += 4 + cmdLen;
    }
}

//
// Replay the early mode journal on the real TPM state. A replay that fails
// would leave PCRs that do not match what the boot loader measured: fail
// closed (the TA stays crashed until the next boot).
//
static void fTPM_replay_journal(void)
{
    uint32_t off = 0;
    uint32_t n = 0;

    while (off + 4 <= fTPM_journal_len) {
        uint32_t len = fTPM_be32(fTPM_journal + off);
        uint32_t respLen = sizeof(fTPM_replay_response);
        uint8_t *respBuf = fTPM_replay_response;

        memcpy(fTPMCommand, fTPM_journal + off + 4, len);
        ExecuteCommand(len, fTPMCommand, &respLen, &respBuf);
        if (fTPMResponseCode(respLen, respBuf) != TPM_RC_SUCCESS) {
            EMSG("fTPM: replay of early PCR extend %u failed", n);
            TEE_Panic(TEE_ERROR_BAD_STATE);
        }
        off += 4 + len;
        n++;
    }
    IMSG("fTPM: replayed %u early PCR extends", n);
    fTPM_journal_len = 0;
    memset(fTPM_journal, 0, sizeof(fTPM_journal));
}

//
// Power up the TPM: load NV (or fall back to a volatile state in early
// mode), manufacture if needed, _TPM_Init and Startup.
//
static void fTPM_power_up(void)
{
    #define STARTUP_SIZE 0x0C

    uint8_t startupClear[STARTUP_SIZE] = { 0x80, 0x01, 0x00, 0x00, 0x00, 0x0c,
                                           0x00, 0x00, 0x01, 0x44, 0x00, 0x00 };
    uint8_t startupState[STARTUP_SIZE] = { 0x80, 0x01, 0x00, 0x00, 0x00, 0x0c,
                                           0x00, 0x00, 0x01, 0x44, 0x00, 0x01 };
    uint32_t respLen;
    uint8_t *respBuf;
    int nvResult;
#ifdef MEASURED_BOOT
    unsigned char tpm_event_log_buf[EVENT_LOG_SIZE];
    size_t tpm_event_log_len = EVENT_LOG_SIZE;
#endif

    // Initialize NV admin state
    _admin__NvInitState();

    // If we fail to open fTPM storage we cannot continue.
    nvResult = _plat__NVEnable(NULL);
    if (nvResult < 0) {
        TEE_Panic(TEE_ERROR_BAD_STATE);
    }
    else if (nvResult > 0) {
        // Storage not reachable (yet): early mode on a volatile state
        fTPM_storage_available = false;
        fTPM_early = true;
        _plat__NvSetVolatile(1);
        IMSG("fTPM: no secure storage: early mode (PCR use only)");
        TPM_Manufacture(1);
    }
    else {
        fTPM_storage_available = true;
        fTPM_early = false;
    }

#ifdef fTPMDebug
    DMSG("NVEnable Complete\n");
#endif

    // This only occurs when there is no previous NV state, i.e., on first
    // boot, after recovering from data loss, we reset the platform, etc.
    if (!fTPM_early && _plat__NvNeedsManufacture()) {
#ifdef fTPMDebug
        DMSG("TPM_Manufacture\n");
#endif
        TPM_Manufacture(1);
    }

    // "Power-On" the platform
    _plat__Signal_PowerOn();

    // Internal init for reference implementation
    _TPM_Init();

#ifdef fTPMDebug
    DMSG("Init Complete\n");
#endif

    // Startup with state
    if (g_chipFlags.fields.TpmStatePresent) {

        // Re-use request buffer for response (ignored)
        respBuf = startupState;
        respLen = STARTUP_SIZE;

        ExecuteCommand(STARTUP_SIZE, startupState, &respLen, &respBuf);
        if (fTPMResponseCode(respLen, respBuf) == TPM_RC_SUCCESS) {
            goto Exit;
        }

#ifdef fTPMDebug
        DMSG("Fall through to startup clear\n");
#endif

        goto Clear;
    }

#ifdef fTPMDebug
    DMSG("No TPM state present\n");
#endif

Clear:
    // Re-use request buffer for response (ignored)
    respBuf = startupClear;
    respLen = STARTUP_SIZE;

    // Fall back to a Startup Clear
    ExecuteCommand(STARTUP_SIZE, startupClear, &respLen, &respBuf);

Exit:
    // Init is complete, indicate so in fTPM admin state.
    g_chipFlags.fields.TpmStatePresent = 1;
    _admin__SaveChipFlags();

    // Initialization complete
    fTPMInitialized = true;

#ifdef MEASURED_BOOT
    // Extend existing TPM Event Log.
    if (get_tpm_event_log(tpm_event_log_buf,
                          &tpm_event_log_len) == TEE_SUCCESS)
    {

#ifdef fTPMDebug
        // Dump the event log
        unsigned char* buff = tpm_event_log_buf;
        size_t buff_len = tpm_event_log_len;
        MSG("Preparing to extend the following TPM Event Log:");
        dump_event_log(tpm_event_log_buf, tpm_event_log_len);
#endif
        process_eventlog(tpm_event_log_buf, tpm_event_log_len);

    }
#endif
}

//
// Leave early mode once secure storage answers.
//
static void fTPM_try_leave_early_mode(void)
{
    if (!fTPM_early || !_plat__NvStorageReachable())
        return;

    IMSG("fTPM: secure storage available: loading the TPM state");
    _plat__NvReset();
    fTPM_power_up();
    if (fTPM_early) {
        // Storage went away again between the check and the load
        EMSG("fTPM: secure storage lost while leaving early mode");
        TEE_Panic(TEE_ERROR_BAD_STATE);
    }
    fTPM_replay_journal();
}

//
// Called when TA instance is created. This is the first call to the TA.
//
TEE_Result TA_CreateEntryPoint(void)
{
#ifdef fTPMDebug
    DMSG("Entry Point\n");
#endif

    // If we've been here before, don't init again.
    if (fTPMInitialized) {
        // We may have had TA_DestroyEntryPoint called but we didn't
        // actually get torn down. Re-NVEnable, just in case.
        if (_plat__NVEnable(NULL)) {
            TEE_Panic(TEE_ERROR_BAD_STATE);
        }
        return TEE_SUCCESS;
    }

    fTPM_power_up();

    return TEE_SUCCESS;
}


//
// Called when TA instance destroyed.  This is the last call in the TA.
//
void TA_DestroyEntryPoint(void)
{
    // We should only see this called after the OS has shutdown and there
    // will be no further commands sent to the TPM. Right now, just close
    // our storage object, becasue the TPM driver should have already
    // shutdown cleanly.
    _plat__NVDisable();
    return;
}


//
// Called when a new session is opened to the TA.
//
TEE_Result TA_OpenSessionEntryPoint(uint32_t    param_types,
                                    TEE_Param   params[4],
                                    void        **sess_ctx)
{
    uint32_t exp_param_types = TA_ALL_PARAM_TYPE(TEE_PARAM_TYPE_NONE);

    // Unreferenced parameters
    UNREFERENCED_PARAMETER(params);
    UNREFERENCED_PARAMETER(sess_ctx);

    // Validate parameter types
    if (param_types != exp_param_types) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // Storage may have become available since the last session (e.g. the
    // OS started its storage service after the boot loader used the TPM)
    fTPM_try_leave_early_mode();

    // Reject sessions when storage is unavailable and not in early mode
    if (!fTPM_storage_available && !fTPM_early) {
        return TEE_ERROR_STORAGE_NOT_AVAILABLE;
    }
    // Only one active session to the fTPM is permitted
    if (fTPMSessionActive) {
        return TEE_ERROR_ACCESS_CONFLICT;
    }

    // Active session
    fTPMSessionActive = true;

    // If return value != TEE_SUCCESS the session will not be created.
    return TEE_SUCCESS;
}


//
// Called when a session is closed.
//
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
    // Unused parameter(s)
    UNREFERENCED_PARAMETER(sess_ctx);

    // Clear active session
    if (fTPMSessionActive) {
        fTPMSessionActive = false;
    }
}

//
// Called to handle command submission.
//
static TEE_Result fTPM_Submit_Command(uint32_t  param_types,
                                      TEE_Param params[4]
)
{
    uint8_t *cmdBuf, *respBuf;
    uint32_t cmdLen, respLen;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INOUT,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    // Validate parameter types
    if (param_types != exp_param_types) {
#ifdef fTPMDebug
        IMSG("Bad param type(s)\n");
#endif
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // Sanity check our buffer sizes
    if ((params[0].memref.size == 0) ||
        (params[1].memref.size == 0) ||
        (params[0].memref.size > MAX_COMMAND_SIZE) ||
        (params[1].memref.size > MAX_RESPONSE_SIZE)) {
#ifdef fTPMDebug
        IMSG("Bad param size(s)\n");
#endif
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // Copy command locally
    memcpy(fTPMCommand, params[0].memref.buffer, params[0].memref.size);

    // Pull the command length from the actual TPM command. The memref size
    // field descibes the buffer containing the command, not the command.
    cmdBuf = fTPMCommand;
    cmdLen = BYTE_ARRAY_TO_UINT32((uint8_t *)&(cmdBuf[2]));

    // Sanity check cmd length included in TPM command
    if (cmdLen > params[0].memref.size) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    respBuf = (uint8_t *)(params[1].memref.buffer);
    respLen = params[1].memref.size;

    fTPM_try_leave_early_mode();

    // Early mode: only commands that need no NV
    if (fTPM_early) {
        fTPM_early_execute(cmdLen, cmdBuf, &respLen, &respBuf);
    }
    // Check if this is a PPI Command
    else if (!_admin__PPICommand(cmdLen, cmdBuf, &respLen, &respBuf)) {
        // If not, pass through to TPM
        ExecuteCommand(cmdLen, cmdBuf, &respLen, &respBuf);
    }

    // Unfortunately, this cannot be done until after we have our response in
    // hand. We will, however, make an effort to return at least a portion of
    // the response along with TEE_ERROR_SHORT_BUFFER.
    if (respLen > params[1].memref.size)
    {
#ifdef fTPMDebug
        IMSG("Insufficient buffer length RS: 0x%x > BL: 0x%x\n", respLen, params[1].memref.size);
#endif
        return TEE_ERROR_SHORT_BUFFER;
    }

#ifdef fTPMDebug
    DMSG("Success, RS: 0x%x\n", respLen);
#endif

    return TEE_SUCCESS;
}

//
// Called to handle PPI commands
//
static TEE_Result fTPM_Emulate_PPI(uint32_t  param_types,
                                   TEE_Param params[4]
)
{
    uint8_t *cmdBuf, *respBuf;
    uint32_t cmdLen, respLen;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                               TEE_PARAM_TYPE_MEMREF_INOUT,
                               TEE_PARAM_TYPE_NONE,
                               TEE_PARAM_TYPE_NONE);

    // Validate parameter types
    if (param_types != exp_param_types) {
#ifdef fTPMDebug
        IMSG("Bad param type(s)\n");
#endif
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // Sanity check our buffer sizes
    if ((params[0].memref.size == 0) ||
        (params[1].memref.size == 0) ||
        (params[0].memref.size > MAX_COMMAND_SIZE) ||
        (params[1].memref.size > MAX_RESPONSE_SIZE)) {
#ifdef fTPMDebug
        IMSG("Bad param size(s)\n");
#endif
        return TEE_ERROR_BAD_PARAMETERS;
    }

    // PPI requests change NV: not possible before storage is available
    fTPM_try_leave_early_mode();
    if (fTPM_early) {
        return TEE_ERROR_STORAGE_NOT_AVAILABLE;
    }

    // Copy command locally
    memcpy(fTPMCommand, params[0].memref.buffer, params[0].memref.size);

    cmdBuf = fTPMCommand;
    cmdLen = params[0].memref.size;

    respBuf = (uint8_t *)(params[1].memref.buffer);
    respLen = params[1].memref.size;

    // Pass along to platform PPI processing
    if (_admin__PPIRequest(cmdLen, cmdBuf, &respLen, &respBuf)) {
#ifdef fTPMDebug
        DMSG("Handled PPI command via TA interface\n");
#endif
    }
    else {
#ifdef fTPMDebug
        IMSG("Failed to handle PPI command via TA interface\n");
#endif
    }

    if (respLen > params[1].memref.size) {
#ifdef fTPMDebug
        IMSG("Insufficient buffer length RS: 0x%x > BL: 0x%x\n", respLen, params[1].memref.size);
#endif
        return TEE_ERROR_SHORT_BUFFER;
    }

    params[1].memref.size = respLen;
    return TEE_SUCCESS;
}

//
// Called when a TA is invoked. Note, paramters come from normal world.
//
TEE_Result TA_InvokeCommandEntryPoint(void      *sess_ctx,
                                      uint32_t  cmd_id,
                                      uint32_t  param_types,
                                      TEE_Param params[4])
{
    // Unused parameter(s)
    UNREFERENCED_PARAMETER(sess_ctx);

    // Handle command invocation
    switch (cmd_id) {

        case TA_FTPM_SUBMIT_COMMAND: {
            return fTPM_Submit_Command(param_types, params);
        }

        case TA_FTPM_EMULATE_PPI: {
            if (!IS_ENABLED(CFG_FTPM_EMULATE_PPI))
                return TEE_ERROR_NOT_SUPPORTED;
            return fTPM_Emulate_PPI(param_types, params);
        }

        default: {
            return TEE_ERROR_BAD_PARAMETERS;
        }
    }
}
