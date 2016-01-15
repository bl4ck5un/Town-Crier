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


#ifndef _NETWORK_RA_H
#define _NETWORK_RA_H


// Enum for all possible message types between the ISV app and
// the ISV SP. Requests and responses in hte remote attestation
// sample.
typedef enum _ra_msg_type_t
{
     TYPE_RA_MSG1,
     TYPE_RA_MSG2,
     TYPE_RA_MSG3,
     TYPE_RA_ATT_RESULT,
}ra_msg_type_t;

// Enum for all possible message types between the SP and IAS.
// Network communication is not simulated in the remote
// attestation sample.  Currently these aren't used.
typedef enum _ias_msg_type_t
{
     TYPE_IAS_ENROLL,
     TYPE_IAS_GET_SIGRL,
     TYPE_IAS_SIGRL,
     TYPE_IAS_ATT_EVIDENCE,
     TYPE_IAS_ATT_RESULT,
}ias_msg_type_t;

#pragma pack(1)
typedef struct _ra_samp_request_header_t{
    uint8_t type;  // set to one of ra_msg_type_t
    uint32_t size; //size of request body,
    uint8_t align[3];
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning ( disable:4200 )
#endif
    uint8_t body[];
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}ra_samp_request_header_t;

typedef struct _ra_samp_response_header_t{
    uint8_t type;   // set to one of ra_msg_type_t
    uint8_t status[2];
    uint32_t size;  //size of the response body
    uint8_t align[1];
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning ( disable:4200 )
#endif
    uint8_t body[];
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}ra_samp_response_header_t;

#pragma pack()

#ifdef  __cplusplus
extern "C" {
#endif

int ra_network_send_receive(const char *server_url,
                            const ra_samp_request_header_t *req,
                            ra_samp_response_header_t **p_resp);
void ra_free_network_response_buffer(ra_samp_response_header_t *resp);

#ifdef  __cplusplus
}
#endif

#endif
