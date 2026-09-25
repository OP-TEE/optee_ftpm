/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2024, Siemens AG
 * All rights reserved.
 * Copyright (c) 2024, Linaro Limited
 *
 * Based on the original code by Microsoft. Modified to support using
 * TEE functions to provide cryptographic functionality.
 *
 * Portions Copyright Microsoft Corporation, see below for details:
 *
 * The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such
 * rights are granted under this license.
 *
 * Copyright (c) 2018 Microsoft Corporation
 *
 * All rights reserved.
 *
 * BSD License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the structure definitions used for ECC in the TEE
 * crypto API version of the code. These definitions would change, based on
 * the library. The ECC-related structures that cross the TPM interface
 * are defined in TpmTypes.h
 */

#ifndef BN_MATH_LIB_DEFINED
#define BN_MATH_LIB_DEFINED

#define BN_MATH_LIB_TEE

/*#define CRYPT_INT_BUF(buftypename, bits)    BN_STRUCT_DEF(buftypename, bits)
#define CRYPT_POINT_BUF(buftypename, bits)  BN_POINT_BUF(buftypename, bits)
#define CRYPT_CURVE_BUF(buftypename, bits)  TPMBN_ECC_CURVE_CONSTANTS*/

typedef struct crypto_impl_description
{
} _CRYPTO_IMPL_DESCRIPTION;

typedef const TPMBN_ECC_CURVE_CONSTANTS*    bigCurveData;

TPM_INLINE const TPMBN_ECC_CURVE_CONSTANTS* AccessCurveConstants(
    const bigCurveData* E)
{
    return *E;
}

#endif /*BN_MATH_LIB_DEFINED*/
