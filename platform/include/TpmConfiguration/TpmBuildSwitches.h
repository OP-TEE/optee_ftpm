/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2026, Siemens AG
 *
 * Copied from the TCG reference implementation, see
 * https://github.com/TrustedComputingGroup/TPM/tree/main/TPMCmd/TpmConfiguration/TpmConfiguration/TpmBuildSwitches.h, tag V184
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

// This file contains the build switches. This contains switches for multiple
// versions of the crypto-library so some may not apply to your environment.
// Each switch has an accompanying description below.
//
// clang-format off
#ifndef _TPM_BUILD_SWITCHES_H_
#define _TPM_BUILD_SWITCHES_H_

#if defined(YES) || defined(NO)
#  error YES and NO should be defined in TpmBuildSwitches.h
#endif
#if defined(SET) || defined(CLEAR)
#  error SET and CLEAR should be defined in TpmBuildSwitches.h
#endif

#define YES   1
#define SET   1
#define NO    0
#define CLEAR 0

// TRUE/FALSE may be coming from system headers, but if not, provide them.
#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

// Need an unambiguous definition for DEBUG. Do not change this
#ifndef DEBUG
#  ifdef NDEBUG
#    define DEBUG NO
#  else
#    define DEBUG YES
#  endif
#elif(DEBUG != NO) && (DEBUG != YES)
#  error DEBUG should be 0 or 1
#endif

////////////////////////////////////////////////////////////////
// DEBUG OPTIONS
////////////////////////////////////////////////////////////////

// The SIMULATION switch allows certain other macros to be enabled. The things that
// can be enabled in a simulation include key caching, reproducible "random"
// sequences, instrumentation of the RSA key generation process, and certain other
// debug code. SIMULATION Needs to be defined as either YES or NO. This grouping of
// macros will make sure that it is set correctly. A simulated TPM would include a
// Virtual TPM. The interfaces for a Virtual TPM should be modified from the standard
// ones in the Simulator project.
#define SIMULATION                  NO

// ENABLE_TPM_DEBUG_PRINT enables arbitrary string printing.
// enables the TPM_DEBUG_PRINT macro to route debugging strings
// to the _plat_debug_out function
#define ENABLE_TPM_DEBUG_PRINT      (YES * SIMULATION)

//  ENABLE_TPM_DEBUG_TRACE enables code tracing macros - depends on TPM_DEBUG_PRINT
#define ENABLE_TPM_DEBUG_TRACE      (NO  * ENABLE_TPM_DEBUG_PRINT)

//  ENABLE_CRYPTO_DEBUG enables printing of actual crypto values. This is entirely insecure.
#define ENABLE_CRYPTO_DEBUG         (YES * ENABLE_TPM_DEBUG_PRINT)

// The CRYPTO_LIB_REPORTING switch allows the TPM to report its
// crypto library implementation, e.g., at simulation startup.
#define CRYPTO_LIB_REPORTING        YES

// If doing debug, can set the DRBG to print out the intermediate test values.
// Before enabling this, make sure that the dbgDumpMemBlock() function
// has been added someplace (preferably, somewhere in CryptRand.c)
#define DRBG_DEBUG_PRINT            (NO  * DEBUG)

// This define is used to control the debug for the CertifyX509 command.
#define CERTIFYX509_DEBUG           (NO * DEBUG)

// This provides fixed seeding of the RNG when doing debug on a simulator. This
// should allow consistent results on test runs as long as the input parameters
// to the functions remains the same.
#define USE_DEBUG_RNG               (NO  * DEBUG)

////////////////////////////////////////////////////////////////
// RSA DEBUG OPTIONS
////////////////////////////////////////////////////////////////

// Enable the instrumentation of the sieve process. This is used to tune the sieve
// variables.
#define RSA_INSTRUMENT              (NO  * DEBUG)

// Enables use of the key cache. Default is YES
#define USE_RSA_KEY_CACHE           (NO  * DEBUG)

// Enables use of a file to store the key cache values so that the TPM will start
// faster during debug. Default for this is YES
#define USE_KEY_CACHE_FILE          (NO  * DEBUG)

////////////////////////////////////////////////////////////////
// TEST OPTIONS
////////////////////////////////////////////////////////////////
// The SIMULATION flag can enable test crypto behaviors and caching that
// significantly change the behavior of the code.  This flag controls only the
// g_forceFailureMode flag in the TPM library while leaving the rest of the TPM
// behavior alone.  Useful for testing when the full set of options controlled by
// SIMULATION may not be desired.
#define ALLOW_FORCE_FAILURE_MODE    NO

////////////////////////////////////////////////////////////////
// Internal checks
////////////////////////////////////////////////////////////////

// Define this to run the function that checks the compatibility between the
// chosen big number math library and the TPM code. Not all ports use this.
// This can be set to NO once https://github.com/microsoft/ms-tpm-20-ref/commit/fcccc82514609f8f61bca69646695303b0197b64
// has been applied to https://github.com/TrustedComputingGroup/TPM/blob/main/TPMCmd/tpm/cryptolibs/TpmBigNum/TpmBigNumThunks.c
#define LIBRARY_COMPATIBILITY_CHECK YES

// In some cases, the relationship between two values may be dependent on things that
// change based on various selections like the chosen cryptographic libraries. It is
// possible that these selections will result in incompatible settings. These are often
// detectable by the compiler but it is not always possible to do the check in the
// preprocessor code. For example, when the check requires use of 'sizeof'() then the
// preprocessor can't do the comparison. For these cases, we include a special macro
// that, depending on the compiler will generate a warning to indicate if the check
// always passes or always fails because it involves fixed constants.
//
// In modern compilers this is now commonly known as a static_assert, but the precise
// implementation varies by compiler. CompilerDependencies.h defines MUST_BE as a macro
// that abstracts out the differences, and COMPILER_CHECKS can remove the checks where
// the current compiler doesn't support it.  COMPILER_CHECKS should be enabled if the
// compiler supports some form of static_assert.
// See the CompilerDependencies_*.h files for specific implementations per compiler.
#ifdef fTPMDebug
#define COMPILER_CHECKS             YES
#else
#define COMPILER_CHECKS             NO
#endif

// Some of the values (such as sizes) are the result of different options set in
// TpmProfile.h. The combination might not be consistent. A function is defined
// (TpmSizeChecks()) that is used to verify the sizes at run time. To enable the
// function, define this parameter.
#ifdef fTPMDebug
#define RUNTIME_SIZE_CHECKS         YES
#else
#define RUNTIME_SIZE_CHECKS         NO
#endif

////////////////////////////////////////////////////////////////
// Compliance options
////////////////////////////////////////////////////////////////

// Enable extra behaviors to meet FIPS compliance requirements
#define FIPS_COMPLIANT              YES

// Indicates if the implementation is to compute the sizes of the proof and primary
// seed size values based on the implemented algorithms.
#define USE_SPEC_COMPLIANT_PROOFS   YES

// Set this to allow compile to continue even though the chosen proof values
// do not match the compliant values. This is written so that someone would
// have to proactively ignore errors.
#define SKIP_PROOF_ERRORS           NO

////////////////////////////////////////////////////////////////
// Implementation alternatives - don't  change external behavior
////////////////////////////////////////////////////////////////
// does the target system have longjmp support, AND we want to use it?
#define LONGJMP_SUPPORTED           NO

// Define TABLE_DRIVEN_DISPATCH to use tables rather than case statements
// for command dispatch and handle unmarshaling
#define TABLE_DRIVEN_DISPATCH       YES

// This define is used to enable the new table-driven marshaling code.
#define TABLE_DRIVEN_MARSHAL        NO

// This switch allows use of #defines in place of pass-through marshaling or
// unmarshaling code. A pass-through function just calls another function to do
// the required function and does no parameter checking of its own. The
// table-driven dispatcher calls directly to the lowest level
// marshaling/unmarshaling code and by-passes any pass-through functions.
#define USE_MARSHALING_DEFINES      YES

// Switch added to support packed lists that leave out space associated with
// unimplemented commands. Comment this out to use linear lists.
// Note: if vendor specific commands are present, the associated list is always
// in compressed form.
#define COMPRESSED_LISTS            YES

// This define is used to eliminate the use of bit-fields. It can be enabled for big-
// or little-endian machines. For big-endian architectures that numbers bits in
// registers from left to right (MSb0) this must be enabled. Little-endian machines
// number from right to left with the least significant bit having assigned a bit
// number of 0. These are LSb0 machines (they are also little-endian so they are also
// least-significant byte 0 (LSB0) machines. Big-endian (MSB0) machines may number in
// either direction (MSb0 or LSb0). For an MSB0+MSb0 machine this value is required to
// be 'NO'
#define USE_BIT_FIELD_STRUCTURES    NO

// Enable the generation of RSA primes using a sieve.
#define RSA_KEY_SIEVE               YES

////////////////////////////////////////////////////////////////
// Implementation alternatives - changes external behavior
////////////////////////////////////////////////////////////////

// This switch enables the RNG state save and restore
#define _DRBG_STATE_SAVE            YES

// Definition to allow alternate behavior for non-orderly startup. If there is a
// chance that the TPM could not update 'failedTries'
#define USE_DA_USED                 YES

// This switch is used to enable the self-test capability in AlgorithmTests.c
#define ENABLE_SELF_TESTS           YES

// This switch indicates where clock epoch value should be stored. If this value
// defined, then it is assumed that the timer will change at any time so the
// nonce should be a random number kept in RAM. When it is not defined, then the
// timer only stops during power outages.
#define CLOCK_STOPS                 NO

// Indicate if the implementation is going to give lockout time credit for time up to
// the last orderly shutdown.
#define ACCUMULATE_SELF_HEAL_TIMER  YES

// If an assertion event is not going to produce any trace information (function and
// line number) then make FAIL_TRACE == NO
#define FAIL_TRACE                  YES

// TODO_RENAME_INC_FOLDER: public refers to the TPM_CoreLib public headers
#include <tpm_public/CompilerDependencies.h>

#endif  // _TPM_BUILD_SWITCHES_H_
