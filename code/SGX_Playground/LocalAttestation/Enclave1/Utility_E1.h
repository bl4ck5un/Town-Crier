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
#ifndef UTILITY_E1_H__
#define UTILITY_E1_H__

#include "stdint.h"

typedef struct _internal_param_struct_t
{
    uint32_t ivar1;
    uint32_t ivar2;
}internal_param_struct_t;

typedef struct _external_param_struct_t
{
    uint32_t var1;
    uint32_t var2;
    internal_param_struct_t *p_internal_struct;
}external_param_struct_t;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t marshal_input_parameters_e2_foo1(uint32_t target_fn_id, uint32_t msg_type, uint32_t var1, uint32_t var2, char** marshalled_buff, size_t* marshalled_buff_len);
uint32_t unmarshal_retval_and_output_parameters_e2_foo1(char* out_buff, char** retval);
uint32_t unmarshal_input_parameters_e1_foo1(external_param_struct_t *pstruct, ms_in_msg_exchange_t* ms);
uint32_t marshal_retval_and_output_parameters_e1_foo1(char** resp_buffer, size_t* resp_length, uint32_t retval, external_param_struct_t *p_struct_var, size_t len_data, size_t len_ptr_data);
uint32_t marshal_message_exchange_request(uint32_t target_fn_id, uint32_t msg_type, uint32_t secret_data, char** marshalled_buff, size_t* marshalled_buff_len);
uint32_t umarshal_message_exchange_request(uint32_t* inp_secret_data, ms_in_msg_exchange_t* ms);
uint32_t marshal_message_exchange_response(char** resp_buffer, size_t* resp_length, uint32_t secret_response);
uint32_t umarshal_message_exchange_response(char* out_buff, char** secret_response);
#ifdef __cplusplus
 }
#endif
#endif