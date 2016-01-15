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


#include <stdlib.h>
#include <string.h>
#include "ecp.h"

#include "sample_libcrypto.h"

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#define MAC_KEY_SIZE       16

#ifndef _MSC_VER
errno_t memcpy_s(
    void *dest,
    size_t numberOfElements,
    const void *src,
    size_t count)
{
    if(numberOfElements<count)
        return -1;
    memcpy(dest, src, count);
    return 0;
}
#endif

bool verify_cmac128(
    sample_ec_key_128bit_t mac_key,
    const uint8_t *p_data_buf,
    uint32_t buf_size,
    const uint8_t *p_mac_buf)
{
    uint8_t data_mac[SAMPLE_EC_MAC_SIZE];
    sample_status_t sample_ret;

    sample_ret = sample_rijndael128_cmac_msg((sample_cmac_128bit_key_t*)mac_key,
        p_data_buf,
        buf_size,
        (sample_cmac_128bit_tag_t *)data_mac);
    if(sample_ret != SAMPLE_SUCCESS)
        return false;
    // In real implementation, should use a time safe version of memcmp here,
    // in order to avoid side channel attack.
    if(!memcmp(p_mac_buf, data_mac, SAMPLE_EC_MAC_SIZE))
        return true;
    return false;
}

typedef struct _ec_padded_shared_key_t
{
    uint8_t s[SAMPLE_ECP_KEY_SIZE];
    uint8_t padding;
} ec_padded_shared_key_t;

bool derive_key(
    const sample_ec_dh_shared_t *p_shared_key,
    uint8_t key_id,
    sample_ec_key_128bit_t derived_key)
{
    sample_status_t sample_ret = SAMPLE_SUCCESS;
    uint8_t cmac_key[MAC_KEY_SIZE];
    ec_padded_shared_key_t pad_key;

    memset(cmac_key, 0, MAC_KEY_SIZE);
    if(memcpy_s(&(pad_key.s), sizeof(sample_ec_dh_shared_t),
                p_shared_key, sizeof(sample_ec_dh_shared_t)))
    {
        return false;
    }
    pad_key.padding = key_id;

    sample_ret = sample_rijndael128_cmac_msg(
        (sample_cmac_128bit_key_t *)cmac_key,
        (uint8_t*)&pad_key,
        sizeof(ec_padded_shared_key_t),
        (sample_cmac_128bit_tag_t *)derived_key);
    // memset heer can be optimized away by compiler, so please use memset_s on
    // windows for production code and similar functions on other OSes.
    memset(&pad_key, 0, sizeof(pad_key));
    if(sample_ret != SAMPLE_SUCCESS)
    {
        return false;
    }
    return true;
}
