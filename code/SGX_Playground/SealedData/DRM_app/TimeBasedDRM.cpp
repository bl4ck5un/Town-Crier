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


#include "stdafx.h"
#include "TimeBasedDRM.h"
#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "DRM_enclave_u.h"
#include <iostream>
using namespace std;

#ifdef __GNUC__
#define ENCLAVE_NAME    "DRM_enclave.signed.so"
#else
#define ENCLAVE_NAME   _T("DRM_enclave.signed.dll")
#endif

TimeBasedDRM::TimeBasedDRM(void): enclave_id(0)
{
    int updated = 0;
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    sgx_ret = sgx_create_enclave(ENCLAVE_NAME, SGX_DEBUG_FLAG,
        &launch_token, &updated, &enclave_id, NULL);
    if (sgx_ret)
    {
        cerr<<"cannot create enclave, error code = 0x"<< hex<< sgx_ret <<endl;
    }
}


TimeBasedDRM::~TimeBasedDRM(void)
{
    if(enclave_id)
        sgx_destroy_enclave(enclave_id);
}

uint32_t TimeBasedDRM:: init(uint8_t*  stored_time_based_policy)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    sgx_ps_cap_t ps_cap;
    memset(&ps_cap, 0, sizeof(sgx_ps_cap_t));
    sgx_ret = sgx_get_ps_cap(&ps_cap);
    if (sgx_ret)
    {
        cerr<<"cannot get platform service capability, error code = 0x"<< hex<<
            sgx_ret <<endl;
        return sgx_ret;
    }
    if (!SGX_IS_TRUSTED_TIME_AVAILABLE(ps_cap))
    {
        cerr<<"trusted time is not supported"<<endl;
        return SGX_ERROR_SERVICE_UNAVAILABLE;
    }
    uint32_t enclave_ret = 0;
    sgx_ret = create_time_based_policy(enclave_id, &enclave_ret,
        (uint8_t *)stored_time_based_policy, time_based_policy_length);
    if (sgx_ret)
    {
        cerr<<"call create_time_based_policy fail, error code = 0x"<< hex<<
            sgx_ret <<endl;
        return sgx_ret;
    } 
    if (enclave_ret)
    {
        cerr<<"cannot create_time_based_policy, function return fail, error code = 0x"
            << hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}


uint32_t TimeBasedDRM:: init()
{
    return init(time_based_policy);
}



uint32_t TimeBasedDRM::perform_function(uint8_t* stored_time_based_policy)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    uint32_t enclave_ret = 0;
    sgx_ret = perform_time_based_policy(enclave_id, &enclave_ret,
        stored_time_based_policy, time_based_policy_length);
    if (sgx_ret)
    {
        cerr<<"call perform_time_based_policy fail, error code = 0x"<< hex<<
            sgx_ret <<endl;
        return sgx_ret;
    }
    if (enclave_ret)
    {
        cerr<<"cannot perform_time_based_policy, function return fail, error code = 0x"
            << hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}

uint32_t TimeBasedDRM::perform_function()
{
    return perform_function(time_based_policy);
}
