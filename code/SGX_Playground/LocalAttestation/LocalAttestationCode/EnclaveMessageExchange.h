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

#include "datatypes.h"
#include "sgx_eid.h"
#include "sgx_trts.h"
#include <map>
#include "dh_session_protocol.h"

#ifndef LOCALATTESTATION_H_
#define LOCALATTESTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

uint32_t SGXAPI create_session(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, dh_session_t *p_session_info);
uint32_t SGXAPI send_request_receive_response(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id, dh_session_t *p_session_info, char *inp_buff, size_t inp_buff_len, size_t max_out_buff_size, char **out_buff, size_t* out_buff_len);
uint32_t SGXAPI close_session(sgx_enclave_id_t src_enclave_id, sgx_enclave_id_t dest_enclave_id);

#ifdef __cplusplus
}
#endif

#endif
