# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2024, Linaro Limited
# Copyright (c) 2026, Siemens AG
#

CFG_FTPM_EMULATE_PPI ?= n
CFG_FTPM_TA_TEE_STORAGE_ID ?= TEE_STORAGE_PRIVATE

#
# The fTPM needs to overwrite some of the header files used in the
# reference implementation. The search order GCC uses is dependent on the
# order the '-I/include/path' arguments are passed in. This is depended on
# the optee_os build system which makes it brittle. Force including these
# files here will make sure the correct files are used first.
#

#cppflags-y += -include platform/include/Platform.h

cppflags-y += -DHASH_LIB=MBEDTLS -DSYM_LIB=TEE -DMATH_LIB=TpmBigNum -DBN_MATH_LIB=TEE
cppflags-y += -D_ARM_
cppflags-y += -DGCC -DVTPM
ifeq ($(CFG_TA_DEBUG),y)
cppflags-y += -DfTPMDebug
cppflags-y += -DTRACE_LEVEL=$(CFG_TEE_TA_LOG_LEVEL)
endif

# too many warnings with ms-tpm-20-ref
cppflags-y += -Wno-strict-aliasing

global-incdirs-y += include
global-incdirs-y += reference/include
global-incdirs-y += platform/include

global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/include
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/include/tpm_public
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/include/private
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/include/private/prototypes
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/cryptolibs/common/include
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/Platform/include
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/TpmConfiguration
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/TpmConfiguration/TpmConfiguration
global-incdirs_ext-y += $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/cryptolibs/TpmBigNum/include/

cflags-y += -fno-strict-aliasing
cflags-y += -Wno-cast-align
cflags-y += -Wno-implicit-fallthrough
cflags-y += -Wno-cast-function-type
cflags-y += -Wno-suggest-attribute=noreturn
cflags-y += -Wno-switch-default
cflags-y += -Wno-redundant-decls
cflags-y += -Wno-strict-prototypes
cflags-y += -Wno-undef

cflags-platform/NVMem.c-y += -Wno-shadow
cflags-platform/NVMem.c-y += -Wno-incompatible-pointer-types
cflags-platform/NVMem.c-y += -Wno-declaration-after-statement
cflags-platform/NVMem.c-y += -Wno-old-style-definition
cflags-platform/NVMem.c-y += -Wno-nested-externs
cflags-platform/NVMem.c-y += -Wno-implicit-function-declaration
cflags-platform/NVMem.c-y += -Wno-missing-declarations
cflags-platform/NVMem.c-y += -Wno-missing-prototypes
cflags-platform/NvAdmin.c-y += -Wno-old-style-definition
cflags-platform/NvAdmin.c-y += -Wno-nested-externs
cflags-platform/NvAdmin.c-y += -Wno-implicit-function-declaration
cflags-platform/NvAdmin.c-y += -Wno-missing-prototypes
cflags-platform/Clock.c-y += -Wno-unused-variable
cflags-fTPM.c-y += -Wno-nested-externs
cflags-fTPM.c-y += -Wno-implicit-function-declaration
cflags-fTPM.c-y += -Wno-unused-variable
cflags-fTPM.c-y += -Wno-incompatible-pointer-types
cflags-fTPM.c-y += -Wno-pointer-arith
cflags-fTPM.c-y += -Wno-format-truncation
cflags-platform/AdminPPI.c-y += -Wno-missing-declarations
cflags-platform/AdminPPI.c-y += -Wno-missing-prototypes
cflags-platform/AdminPPI.c-y += -Wno-unknown-pragmas
cflags-platform/PlatformACT.c-y += -Wno-missing-declarations
cflags-platform/PlatformACT.c-y += -Wno-missing-prototypes
cflags-platform/fTPM_helpers.c-y += -Wno-missing-declarations
cflags-platform/fTPM_helpers.c-y += -Wno-missing-prototypes
cflags-platform/fTPM_event_log.c-y += -Wno-incompatible-pointer-types
cflags-platform/EventLogPrint.c-y += -Wno-pointer-arith
cflags-platform/EventLogPrint.c-y += -Wno-format-truncation
cflags-platform/EventLogPrint.c-y += -Wno-restrict

cppflags-y += -DCFG_FTPM_TA_TEE_STORAGE_ID=$(CFG_FTPM_TA_TEE_STORAGE_ID)

srcs-y += platform/AdminPPI.c
srcs-y += platform/Cancel.c
srcs-y += platform/Clock.c
srcs-y += platform/Entropy.c
srcs-y += platform/ExtraData.c
srcs-y += platform/LocalityPlat.c
srcs-y += platform/NvAdmin.c
srcs-y += platform/NVMem.c
srcs-y += platform/PowerPlat.c
srcs-y += platform/PlatformData.c
srcs-y += platform/PlatformPcr.c
srcs-y += platform/PPPlat.c
srcs-y += platform/RunCommand.c
srcs-y += platform/PlatformACT.c
srcs-y += platform/fTPM_helpers.c
srcs-y += platform/VendorInfo.c
srcs-y += platform/Failure.c
srcs-y += platform/Init.c
srcs-y += platform/SelfTest.c

srcs-y += fTPM.c

ifeq ($(CFG_TA_MEASURED_BOOT),y)
CFG_TA_EVENT_LOG_SIZE ?= 1024
# Support for Trusted Firmware Measured Boot.
srcs-y += platform/fTPM_event_log.c
srcs-y += platform/EventLogPrint.c
cppflags-y += -DEVENT_LOG_SIZE=$(CFG_TA_EVENT_LOG_SIZE)
cppflags-y += -DMEASURED_BOOT
endif


srcs-y += tee/BnToTEEMath.c
srcs-y += tee/BnToTEESupport.c
srcs-y += tee/TpmToTEESym.c

srcs_ext_base-y := $(CFG_MS_TPM_20_REF)/TPMCmd/tpm/
srcs_ext-y += src/X509/X509_ECC.c
srcs_ext-y += src/X509/X509_RSA.c
srcs_ext-y += src/X509/TpmASN1.c
srcs_ext-y += src/X509/X509_spt.c
srcs_ext-y += src/command/Attestation/CertifyX509.c
srcs_ext-y += src/command/Attestation/GetCommandAuditDigest.c
srcs_ext-y += src/command/Attestation/GetSessionAuditDigest.c
srcs_ext-y += src/command/Attestation/Attest_spt.c
srcs_ext-y += src/command/Attestation/Quote.c
srcs_ext-y += src/command/Attestation/Certify.c
srcs_ext-y += src/command/Attestation/CertifyCreation.c
srcs_ext-y += src/command/Attestation/GetTime.c
srcs_ext-y += src/command/Random/GetRandom.c
srcs_ext-y += src/command/Random/StirRandom.c
srcs_ext-y += src/command/NVStorage/NV_WriteLock.c
srcs_ext-y += src/command/NVStorage/NV_ReadPublic.c
srcs_ext-y += src/command/NVStorage/NV_spt.c
srcs_ext-y += src/command/NVStorage/NV_Increment.c
srcs_ext-y += src/command/NVStorage/NV_ChangeAuth.c
srcs_ext-y += src/command/NVStorage/NV_UndefineSpaceSpecial.c
srcs_ext-y += src/command/NVStorage/NV_SetBits.c
srcs_ext-y += src/command/NVStorage/NV_Write.c
srcs_ext-y += src/command/NVStorage/NV_GlobalWriteLock.c
srcs_ext-y += src/command/NVStorage/NV_Read.c
srcs_ext-y += src/command/NVStorage/NV_Extend.c
srcs_ext-y += src/command/NVStorage/NV_Certify.c
srcs_ext-y += src/command/NVStorage/NV_ReadLock.c
srcs_ext-y += src/command/NVStorage/NV_DefineSpace.c
srcs_ext-y += src/command/NVStorage/NV_UndefineSpace.c
srcs_ext-y += src/command/HashHMAC/HashSequenceStart.c
srcs_ext-y += src/command/HashHMAC/SequenceUpdate.c
srcs_ext-y += src/command/HashHMAC/MAC_Start.c
srcs_ext-y += src/command/HashHMAC/EventSequenceComplete.c
srcs_ext-y += src/command/HashHMAC/HMAC_Start.c
srcs_ext-y += src/command/HashHMAC/SequenceComplete.c
srcs_ext-y += src/command/Ecdaa/Commit.c
srcs_ext-y += src/command/Startup/Startup.c
srcs_ext-y += src/command/Startup/Shutdown.c
srcs_ext-y += src/command/FieldUpgrade/FieldUpgradeData.c
srcs_ext-y += src/command/FieldUpgrade/FirmwareRead.c
srcs_ext-y += src/command/FieldUpgrade/FieldUpgradeStart.c
srcs_ext-y += src/command/Capability/TestParms.c
srcs_ext-y += src/command/Capability/GetCapability.c
srcs_ext-y += src/command/ClockTimer/ACT_spt.c
srcs_ext-y += src/command/ClockTimer/ClockRateAdjust.c
srcs_ext-y += src/command/ClockTimer/ACT_SetTimeout.c
srcs_ext-y += src/command/ClockTimer/ClockSet.c
srcs_ext-y += src/command/ClockTimer/ReadClock.c
srcs_ext-y += src/command/Session/PolicyRestart.c
srcs_ext-y += src/command/Session/StartAuthSession.c
srcs_ext-y += src/command/EA/PolicyDuplicationSelect.c
srcs_ext-y += src/command/EA/PolicyPCR.c
srcs_ext-y += src/command/EA/PolicySecret.c
srcs_ext-y += src/command/EA/PolicyTicket.c
srcs_ext-y += src/command/EA/PolicyTemplate.c
srcs_ext-y += src/command/EA/PolicyNV.c
srcs_ext-y += src/command/EA/PolicyGetDigest.c
srcs_ext-y += src/command/EA/PolicyCpHash.c
srcs_ext-y += src/command/EA/PolicyOR.c
srcs_ext-y += src/command/EA/Policy_spt.c
srcs_ext-y += src/command/EA/PolicyLocality.c
srcs_ext-y += src/command/EA/PolicyAuthorize.c
srcs_ext-y += src/command/EA/PolicyAuthorizeNV.c
srcs_ext-y += src/command/EA/PolicyPassword.c
srcs_ext-y += src/command/EA/PolicyCounterTimer.c
srcs_ext-y += src/command/EA/PolicyAuthValue.c
srcs_ext-y += src/command/EA/PolicySigned.c
srcs_ext-y += src/command/EA/PolicyNameHash.c
srcs_ext-y += src/command/EA/PolicyNvWritten.c
srcs_ext-y += src/command/EA/PolicyPhysicalPresence.c
srcs_ext-y += src/command/EA/PolicyCommandCode.c
srcs_ext-y += src/command/Hierarchy/ChangePPS.c
srcs_ext-y += src/command/Hierarchy/HierarchyControl.c
srcs_ext-y += src/command/Hierarchy/HierarchyChangeAuth.c
srcs_ext-y += src/command/Hierarchy/ChangeEPS.c
srcs_ext-y += src/command/Hierarchy/ClearControl.c
srcs_ext-y += src/command/Hierarchy/Clear.c
srcs_ext-y += src/command/Hierarchy/SetPrimaryPolicy.c
srcs_ext-y += src/command/Hierarchy/CreatePrimary.c
srcs_ext-y += src/command/CommandAudit/SetCommandCodeAuditStatus.c
srcs_ext-y += src/command/Object/Object_spt.c
srcs_ext-y += src/command/Object/ReadPublic.c
srcs_ext-y += src/command/Object/Load.c
srcs_ext-y += src/command/Object/LoadExternal.c
srcs_ext-y += src/command/Object/MakeCredential.c
srcs_ext-y += src/command/Object/Unseal.c
srcs_ext-y += src/command/Object/CreateLoaded.c
srcs_ext-y += src/command/Object/ObjectChangeAuth.c
srcs_ext-y += src/command/Object/ActivateCredential.c
srcs_ext-y += src/command/Object/Create.c
srcs_ext-y += src/command/AttachedComponent/AC_GetCapability.c
srcs_ext-y += src/command/AttachedComponent/AC_spt.c
srcs_ext-y += src/command/AttachedComponent/AC_Send.c
srcs_ext-y += src/command/AttachedComponent/Policy_AC_SendSelect.c
srcs_ext-y += src/command/Signature/VerifySignature.c
srcs_ext-y += src/command/Signature/Sign.c
srcs_ext-y += src/command/Duplication/Import.c
srcs_ext-y += src/command/Duplication/Rewrap.c
srcs_ext-y += src/command/Duplication/Duplicate.c
srcs_ext-y += src/command/EA/PolicyCapability.c
srcs_ext-y += src/command/EA/PolicyParameters.c
srcs_ext-y += src/command/Symmetric/EncryptDecrypt2.c
srcs_ext-y += src/command/Symmetric/EncryptDecrypt_spt.c
srcs_ext-y += src/command/Symmetric/HMAC.c
srcs_ext-y += src/command/Symmetric/Hash.c
srcs_ext-y += src/command/Symmetric/EncryptDecrypt.c
srcs_ext-y += src/command/Symmetric/MAC.c
srcs_ext-y += src/command/Context/ContextSave.c
srcs_ext-y += src/command/Context/FlushContext.c
srcs_ext-y += src/command/Context/Context_spt.c
srcs_ext-y += src/command/Context/ContextLoad.c
srcs_ext-y += src/command/Context/EvictControl.c
srcs_ext-y += src/command/PCR/PCR_Reset.c
srcs_ext-y += src/command/PCR/PCR_Allocate.c
srcs_ext-y += src/command/PCR/PCR_Extend.c
srcs_ext-y += src/command/PCR/PCR_SetAuthValue.c
srcs_ext-y += src/command/PCR/PCR_Event.c
srcs_ext-y += src/command/PCR/PCR_SetAuthPolicy.c
srcs_ext-y += src/command/PCR/PCR_Read.c
srcs_ext-y += src/command/DA/DictionaryAttackParameters.c
srcs_ext-y += src/command/DA/DictionaryAttackLockReset.c
srcs_ext-y += src/command/Misc/PP_Commands.c
srcs_ext-y += src/command/Misc/SetAlgorithmSet.c
srcs_ext-y += src/command/NVStorage/NV_DefineSpace2.c
srcs_ext-y += src/command/NVStorage/NV_ReadPublic2.c
srcs_ext-y += src/command/Testing/GetTestResult.c
srcs_ext-y += src/command/Testing/SelfTest.c
srcs_ext-y += src/command/Testing/IncrementalSelfTest.c
srcs_ext-y += src/command/Asymmetric/ECC_Parameters.c
srcs_ext-y += src/command/Asymmetric/RSA_Encrypt.c
srcs_ext-y += src/command/Asymmetric/ECDH_ZGen.c
srcs_ext-y += src/command/Asymmetric/ECDH_KeyGen.c
srcs_ext-y += src/command/Asymmetric/ZGen_2Phase.c
srcs_ext-y += src/command/Asymmetric/ECC_Decrypt.c
srcs_ext-y += src/command/Asymmetric/RSA_Decrypt.c
srcs_ext-y += src/command/Asymmetric/EC_Ephemeral.c
srcs_ext-y += src/command/Asymmetric/ECC_Encrypt.c
srcs_ext-y += src/subsystem/DA.c
srcs_ext-y += src/subsystem/NvDynamic.c
srcs_ext-y += src/subsystem/Object.c
srcs_ext-y += src/subsystem/PP.c
srcs_ext-y += src/subsystem/Session.c
srcs_ext-y += src/subsystem/NvReserved.c
srcs_ext-y += src/subsystem/Hierarchy.c
srcs_ext-y += src/subsystem/Time.c
srcs_ext-y += src/subsystem/PCR.c
srcs_ext-y += src/subsystem/CommandAudit.c
srcs_ext-y += src/events/_TPM_Hash_Start.c
srcs_ext-y += src/events/_TPM_Init.c
srcs_ext-y += src/events/_TPM_Hash_Data.c
srcs_ext-y += src/events/_TPM_Hash_End.c
srcs_ext-y += src/crypt/CryptSmac.c
srcs_ext-y += src/crypt/CryptEccData.c
srcs_ext-y += src/crypt/CryptCmac.c
srcs_ext-y += src/crypt/CryptEccSignature.c
srcs_ext-y += src/crypt/AlgorithmTests.c
srcs_ext-y += src/crypt/CryptSelfTest.c
srcs_ext-y += src/crypt/Ticket.c
srcs_ext-y += src/crypt/CryptPrimeSieve.c
srcs_ext-y += src/crypt/CryptEccKeyExchange.c
srcs_ext-y += src/crypt/CryptRand.c
srcs_ext-y += src/crypt/CryptEccMain.c
srcs_ext-y += src/crypt/CryptSym.c
srcs_ext-y += src/crypt/RsaKeyCache.c
srcs_ext-y += src/crypt/CryptUtil.c
srcs_ext-y += src/crypt/CryptEccCrypt.c
srcs_ext-y += src/crypt/CryptRsa.c
srcs_ext-y += src/crypt/CryptPrime.c
srcs_ext-y += src/crypt/PrimeData.c
srcs_ext-y += src/crypt/CryptHash.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Signature_ECDAA.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Signature_ECDSA.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Signature_Schnorr.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Signature_SM2.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Signature_Util.c
srcs_ext-y += src/crypt/ecc/TpmEcc_Util.c
srcs_ext-y += src/crypt/math/TpmMath_Util.c
srcs_ext-y += src/support/Marshal.c
srcs_ext-y += src/support/MathOnByteBuffers.c
srcs_ext-y += src/support/TableDrivenMarshal.c
srcs_ext-y += src/support/PropertyCap.c
srcs_ext-y += src/support/Locality.c
srcs_ext-y += src/support/TableMarshalData.c
srcs_ext-y += src/support/Memory.c
srcs_ext-y += src/support/Response.c
srcs_ext-y += src/support/ResponseCodeProcessing.c
srcs_ext-y += src/support/Global.c
srcs_ext-y += src/support/Power.c
srcs_ext-y += src/support/AlgorithmCap.c
srcs_ext-y += src/support/CommandCodeAttributes.c
srcs_ext-y += src/support/Entity.c
srcs_ext-y += src/support/Handle.c
srcs_ext-y += src/support/TpmFail.c
srcs_ext-y += src/support/TpmSizeChecks.c
srcs_ext-y += src/support/Manufacture.c
srcs_ext-y += src/support/IoBuffers.c
srcs_ext-y += src/support/Bits.c
srcs_ext-y += src/main/SessionProcess.c
srcs_ext-y += src/main/CommandDispatcher.c
srcs_ext-y += src/main/ExecCommand.c
srcs_ext-y += cryptolibs/TpmBigNum/BnConvert.c
srcs_ext-y += cryptolibs/TpmBigNum/BnEccConstants.c
srcs_ext-y += cryptolibs/TpmBigNum/BnMath.c
srcs_ext-y += cryptolibs/TpmBigNum/BnMemory.c
srcs_ext-y += cryptolibs/TpmBigNum/TpmBigNumThunks.c
srcs_ext-y += ../TpmConfiguration/TpmVendorCommandHandlers/Vendor_TCG_Test.c
srcs_ext-y += ../Platform/src/NVVirtual.c
