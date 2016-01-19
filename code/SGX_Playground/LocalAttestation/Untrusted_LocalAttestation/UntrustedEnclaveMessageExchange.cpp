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

#include "sgx_eid.h"
#include "error_codes.h"
#include "datatypes.h"
#include "sgx_urts.h"
#include "UntrustedEnclaveMessageExchange.h"
#include "sgx_dh.h"
#include <map>

std::map<sgx_enclave_id_t, uint32_t>g_enclave_id_map;

//Makes an sgx_ecall to the destination enclave to get session id and message1
ATTESTATION_STATUS session_request_ocall(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, sgx_dh_msg1_t* dh_msg1, uint32_t* session_id)
{
    uint32_t status = 0;
    sgx_status_t ret = SGX_SUCCESS;
    uint32_t temp_enclave_no;

    std::map<sgx_enclave_id_t, uint32_t>::iterator it = g_enclave_id_map.find(dest_enclave_id);
    if(it != g_enclave_id_map.end())
    {
        temp_enclave_no = it->second;
    }
    else
    {
        return INVALID_SESSION;
    }

    switch(temp_enclave_no)
    {
        case 1:
            ret = Enclave1_session_request(dest_enclave_id, &status, src_enclave_id, dh_msg1, session_id);
            break;
        case 2:
            ret = Enclave2_session_request(dest_enclave_id, &status, src_enclave_id, dh_msg1, session_id);
            break;
        case 3:
            ret = Enclave3_session_request(dest_enclave_id, &status, src_enclave_id, dh_msg1, session_id);
            break;
    }
    if (ret == SGX_SUCCESS)
        return (ATTESTATION_STATUS)status;
    else	
        return INVALID_SESSION;

}
//Makes an sgx_ecall to the destination enclave sends message2 from the source enclave and gets message 3 from the destination enclave
ATTESTATION_STATUS exchange_report_ocall(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, sgx_dh_msg2_t *dh_msg2, sgx_dh_msg3_t *dh_msg3, uint32_t session_id)
{
    uint32_t status = 0;
    sgx_status_t ret = SGX_SUCCESS;
    uint32_t temp_enclave_no;

    std::map<sgx_enclave_id_t, uint32_t>::iterator it = g_enclave_id_map.find(dest_enclave_id);
    if(it != g_enclave_id_map.end())
    {
        temp_enclave_no = it->second;
    }
    else
    {
        return INVALID_SESSION;
    }

    switch(temp_enclave_no)
    {
        case 1:
            ret = Enclave1_exchange_report(dest_enclave_id, &status, src_enclave_id, dh_msg2, dh_msg3, session_id);
            break;
        case 2:
            ret = Enclave2_exchange_report(dest_enclave_id, &status, src_enclave_id, dh_msg2, dh_msg3, session_id);
            break;
        case 3:
            ret = Enclave3_exchange_report(dest_enclave_id, &status, src_enclave_id, dh_msg2, dh_msg3, session_id);
            break;
    }
    if (ret == SGX_SUCCESS)
        return (ATTESTATION_STATUS)status;
    else	
        return INVALID_SESSION;

}

//Make an sgx_ecall to the destination enclave function that generates the actual response
ATTESTATION_STATUS send_request_ocall(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id,secure_message_t* req_message, size_t req_message_size, size_t max_payload_size, secure_message_t* resp_message, size_t resp_message_size)
{
    uint32_t status = 0;
    sgx_status_t ret = SGX_SUCCESS;
    uint32_t temp_enclave_no;

    std::map<sgx_enclave_id_t, uint32_t>::iterator it = g_enclave_id_map.find(dest_enclave_id);
    if(it != g_enclave_id_map.end())
    {
        temp_enclave_no = it->second;
    }
    else
    {
        return INVALID_SESSION;
    }

    switch(temp_enclave_no)
    {
        case 1:
            ret = Enclave1_generate_response(dest_enclave_id, &status, src_enclave_id, req_message, req_message_size, max_payload_size, resp_message, resp_message_size);
            break;
        case 2:
            ret = Enclave2_generate_response(dest_enclave_id, &status, src_enclave_id, req_message, req_message_size, max_payload_size, resp_message, resp_message_size);
            break;
        case 3:
            ret = Enclave3_generate_response(dest_enclave_id, &status, src_enclave_id, req_message, req_message_size, max_payload_size, resp_message, resp_message_size);
            break;
    }
    if (ret == SGX_SUCCESS)
        return (ATTESTATION_STATUS)status;
    else	
        return INVALID_SESSION;

}

//Make an sgx_ecall to the destination enclave to close the session
ATTESTATION_STATUS end_session_ocall(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id)
{
    uint32_t status = 0;
    sgx_status_t ret = SGX_SUCCESS;
    uint32_t temp_enclave_no;

    std::map<sgx_enclave_id_t, uint32_t>::iterator it = g_enclave_id_map.find(dest_enclave_id);
    if(it != g_enclave_id_map.end())
    {
        temp_enclave_no = it->second;
    }
    else
    {
        return INVALID_SESSION;
    }

    switch(temp_enclave_no)
    {
        case 1:
            ret = Enclave1_end_session(dest_enclave_id, &status, src_enclave_id);
            break;
        case 2:
            ret = Enclave2_end_session(dest_enclave_id, &status, src_enclave_id);
            break;
        case 3:
            ret = Enclave3_end_session(dest_enclave_id, &status, src_enclave_id);
            break;
    }
    if (ret == SGX_SUCCESS)
        return (ATTESTATION_STATUS)status;
    else	
        return INVALID_SESSION;

}
