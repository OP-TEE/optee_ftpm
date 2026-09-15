/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Siemens AG
 *
 * Copied from the TCG reference implementation, see
 * https://github.com/TrustedComputingGroup/TPM/tree/main/TPMCmd/Platform/src/ExtraData.c, tag V184
 */
/*
 * TCG Reference Implementation for TPM 2.0
 * This code is informative.
 *
 * The copyright in this software is being made available under the BSD License,
 * included below.
 *
 * Copyright 2010-2022 Microsoft Corporation
 * Copyright 2022-2025 Trusted Computing Group and its contributors
 *
 * All rights reserved.
 *
 * BSD License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS""
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//** Description
//
// This file contains routines that are called by the core library to allow the
// platform to use the Core storage structures for small amounts of related data.
//
// In this implementation, the buffers are all just set to 0xFF

//** Includes and Data Definitions
#include "Tpm.h"
#include "prototypes/platform_public_interface.h"

//** _plat__GetPlatformManufactureData

// This function allows the platform to provide a small amount of data to be
// stored as part of the TPM's PERSISTENT_DATA structure during manufacture.  Of
// course the platform can store data separately as well, but this allows a
// simple platform implementation to store a few bytes of data without
// implementing a multi-layer storage system.  This function is called on
// manufacture and CLEAR.  The buffer will contain the last value provided
// to the Core library.
LIB_EXPORT void _plat__GetPlatformManufactureData(uint8_t* pPlatformPersistentData,
                                                  uint32_t bufferSize)
{
    if(bufferSize != 0)
    {
        memset((void*)pPlatformPersistentData, 0xFF, bufferSize);
    }
}
