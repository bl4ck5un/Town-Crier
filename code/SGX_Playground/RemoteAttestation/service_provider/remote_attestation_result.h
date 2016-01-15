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

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define SAMPLE_MAC_SIZE             16  // Message Authentication Code
                                        // - 16 bytes
typedef uint8_t                     sample_mac_t[SAMPLE_MAC_SIZE];

#ifndef SAMPLE_FEBITSIZE
    #define SAMPLE_FEBITSIZE        256
#endif

#define SAMPLE_NISTP256_KEY_SIZE    (SAMPLE_FEBITSIZE/ 8 /sizeof(uint32_t))

typedef struct sample_ec_sign256_t
{
    uint32_t x[SAMPLE_NISTP256_KEY_SIZE];
    uint32_t y[SAMPLE_NISTP256_KEY_SIZE];
} sample_ec_sign256_t;

#pragma pack(push,1)

#define SAMPLE_SP_TAG_SIZE          16

typedef struct sp_aes_gcm_data_t {
    uint32_t        payload_size;       //  0: Size of the payload which is
                                        //     encrypted
    uint8_t         reserved[12];       //  4: Reserved bits
    uint8_t	        payload_tag[SAMPLE_SP_TAG_SIZE];
                                        // 16: AES-GMAC of the plain text,
                                        //     payload, and the sizes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning ( disable:4200 )
#endif
    uint8_t         payload[];          // 32: Ciphertext of the payload
                                        //     followed by the plain text
#ifdef _MSC_VER
#pragma warning(pop)
#endif
} sp_aes_gcm_data_t;


#define ISVSVN_SIZE 2
#define PSDA_SVN_SIZE 4
#define GID_SIZE 4
#define PSVN_SIZE 18

// @TODO: Modify at production to use the values specified by the Production
// IAS API
typedef struct ias_platform_info_blob_t
{
     uint8_t sample_epid_group_status;
     uint16_t sample_tcb_evaluation_status;
     uint16_t pse_evaluation_status;
     uint8_t latest_equivalent_tcb_psvn[PSVN_SIZE];
     uint8_t latest_pse_isvsvn[ISVSVN_SIZE];
     uint8_t latest_psda_svn[PSDA_SVN_SIZE];
     uint8_t performance_rekey_gid[GID_SIZE];
     sample_ec_sign256_t signature;
} ias_platform_info_blob_t;


typedef struct sample_ra_att_result_msg_t {
    ias_platform_info_blob_t    platform_info_blob;
    sample_mac_t                mac;    // mac_smk(attestation_status)
    sp_aes_gcm_data_t           secret;
} sample_ra_att_result_msg_t;


#pragma pack(pop)


#ifdef  __cplusplus
}
#endif

