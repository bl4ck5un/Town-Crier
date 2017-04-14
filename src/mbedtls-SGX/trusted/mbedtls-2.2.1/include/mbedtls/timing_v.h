/**
 * \file timing.h
 *
 * \brief Portable interface to the CPU cycle counter
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef MBEDTLS_TIMING_V_H
#define MBEDTLS_TIMING_V_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if !defined(MBEDTLS_TIMING_ALT)
// Regular implementation
//

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          timer structure
 */
struct mbedtls_timing_hr_time
{
    unsigned char opaque[32];
};

/**
 * \brief          Context for mbedtls_timing_set/get_delay()
 */
typedef struct
{
    struct mbedtls_timing_hr_time   timer;
    uint32_t                        int_ms;
    uint32_t                        fin_ms;
} mbedtls_timing_delay_context;

extern volatile int mbedtls_timing_alarmed;

#ifdef __cplusplus
}
#endif

#else  /* MBEDTLS_TIMING_ALT */
#include "timing_alt.h"
#endif /* MBEDTLS_TIMING_ALT */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MBEDTLS_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int mbedtls_timing_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#endif /* timing.h */
