/* SPDX-License-Identifier: BSD-2-Clause */
/**********************************************************************
 * Copyright (c) 2024, Siemens AG
 * All rights reserved.
 */

#include "Tpm.h"

#if defined(BN_MATH_LIB_TEE)

LIB_EXPORT int BnSupportLibInit(void)
{
    return TRUE;
}

#endif
