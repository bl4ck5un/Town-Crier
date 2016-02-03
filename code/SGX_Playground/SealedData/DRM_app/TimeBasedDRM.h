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
#include "stdlib.h"
#include "sgx.h"
#include "sgx_urts.h"
#include "../include/sealed_data_defines.h"

class TimeBasedDRM
{
public:
    TimeBasedDRM(void);
    ~TimeBasedDRM(void);
        uint32_t init(uint8_t*  stored_time_based_policy);
    uint32_t init();
    
    uint32_t perform_function();
    uint32_t perform_function(uint8_t* stored_time_based_policy);

    uint32_t get_time_based_policy(uint8_t* stored_time_based_policy);

    static const uint32_t time_based_policy_length = TIME_BASED_PAY_LOAD_SIZE;
private:
    uint8_t  time_based_policy[time_based_policy_length];
    sgx_enclave_id_t enclave_id;
    sgx_launch_token_t launch_token;
};

