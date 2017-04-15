/*
 * (C) Copyright 2016 Fan Zhang (bl4ck5unxx@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 *     Fan Zhang <bl4ck5unxx@gmail.com>
 * 
 * This is the dummy header file for APIs implemented outside of the
 * enclave (i.e. via ecalls).
 */

#pragma once

#define _vsnprintf vsnprintf
#include <stdio.h>      /* vsnprintf */

#if defined(__cplusplus)
extern "C" {
#endif

int mbedtls_sgx_drbg_random( void *p_rng, unsigned char *output, size_t out_len );
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen );
int printf_sgx(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif
