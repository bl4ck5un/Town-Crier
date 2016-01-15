/**
*   Copyright(C) 2011-2015 Intel Corporation All Rights Reserved.
*
*   The source code, information  and  material ("Material") contained herein is
*   owned  by Intel Corporation or its suppliers or licensors, and title to such
*   Material remains  with Intel Corporation  or its suppliers or licensors. The
*   Material  contains proprietary information  of  Intel or  its  suppliers and
*   licensors. The  Material is protected by worldwide copyright laws and treaty
*   provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way  without Intel's  prior  express written  permission. No  license
*   under  any patent, copyright  or  other intellectual property rights  in the
*   Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
*   implication, inducement,  estoppel or  otherwise.  Any  license  under  such
*   intellectual  property  rights must  be express  and  approved  by  Intel in
*   writing.
*
*   *Third Party trademarks are the property of their respective owners.
*
*   Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
*   this  notice or  any other notice embedded  in Materials by Intel or Intel's
*   suppliers or licensors in any way.
*/


#pragma once

#ifndef _ECP_H
#define _ECP_H

#include <stdint.h>
#include <stdlib.h>

#include "remote_attestation_result.h"

#ifndef SAMPLE_FEBITSIZE
    #define SAMPLE_FEBITSIZE                    256
#endif

#define SAMPLE_ECP_KEY_SIZE                     (SAMPLE_FEBITSIZE/8)

typedef struct sample_ec_priv_t
{
    uint8_t r[SAMPLE_ECP_KEY_SIZE];
} sample_ec_priv_t;

typedef struct sample_ec_dh_shared_t
{
    uint8_t s[SAMPLE_ECP_KEY_SIZE];
}sample_ec_dh_shared_t;

typedef uint8_t sample_ec_key_128bit_t[16];

#define SAMPLE_EC_MAC_SIZE 16

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
typedef int errno_t;
#endif
errno_t memcpy_s(void *dest, size_t numberOfElements, const void *src,
                 size_t count);
#endif

bool derive_key(
    const sample_ec_dh_shared_t *p_share_key,
    uint8_t key_id,
    sample_ec_key_128bit_t derived_key);

bool verify_cmac128(
    sample_ec_key_128bit_t mac_key,
    const uint8_t *p_data_buf,
    uint32_t buf_size,
    const uint8_t *p_mac_buf);
#ifdef  __cplusplus
}
#endif

#endif

