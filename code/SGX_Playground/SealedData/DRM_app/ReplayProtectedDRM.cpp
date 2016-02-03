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
#ifdef _MSC_VER
#include <tchar.h>
#endif
#include <iostream>
#include "sgx.h"
#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "DRM_enclave_u.h"
#include "ReplayProtectedDRM.h"
#include "sgx_tseal.h"

using namespace std;

#ifdef __GNUC__
#define ENCLAVE_NAME    "DRM_enclave.signed.so"
#else
#define ENCLAVE_NAME   _T("DRM_enclave.signed.dll")
#endif


ReplayProtectedDRM::ReplayProtectedDRM(): enclave_id(0)
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


ReplayProtectedDRM::~ReplayProtectedDRM(void)
{
    if(enclave_id)
        sgx_destroy_enclave(enclave_id);
}

uint32_t ReplayProtectedDRM:: init(uint8_t*  stored_sealed_activity_log)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    sgx_ps_cap_t ps_cap;
    memset(&ps_cap, 0, sizeof(sgx_ps_cap_t));
    sgx_ret = sgx_get_ps_cap(&ps_cap);
    if (sgx_ret)
    {
        cerr<<"cannot get platform service capability, error code = 0x"<< hex
            << sgx_ret <<endl;
        return sgx_ret;
    }
    if (!SGX_IS_MONOTONIC_COUNTER_AVAILABLE(ps_cap))
    {
        cerr<<"monotonic counter is not supported"<<endl;
        return SGX_ERROR_SERVICE_UNAVAILABLE;
    }
    uint32_t enclave_ret = 0;
    sgx_ret = create_sealed_policy(enclave_id, &enclave_ret,
        (uint8_t *)stored_sealed_activity_log, sealed_activity_log_length);
    if (sgx_ret)
    {
        cerr<<"call create_sealed_policy fail, error code = 0x"<< hex<< sgx_ret
            <<endl;
        return sgx_ret;
    } 
    if (enclave_ret)
    {
        cerr<<"cannot create_sealed_policy, function return fail, error code ="
            "0x"<< hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}


uint32_t ReplayProtectedDRM:: init()
{
    return init(sealed_activity_log);
}



uint32_t ReplayProtectedDRM:: perform_function(
    uint8_t* stored_sealed_activity_log)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    uint32_t enclave_ret = 0;
    sgx_ret = perform_sealed_policy(enclave_id, &enclave_ret,
        (uint8_t *)stored_sealed_activity_log, sealed_activity_log_length);
    if (sgx_ret)
    {
        cerr<<"call perform_sealed_policy fail, error code = 0x"<< hex<< sgx_ret
            <<endl;
        return sgx_ret;
    } 
    if (enclave_ret)
    {
        cerr<<"cannot perform_sealed_policy, function return fail, error code ="
            "0x"<< hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}

uint32_t ReplayProtectedDRM:: perform_function()
{
    return perform_function(sealed_activity_log);
}

uint32_t ReplayProtectedDRM:: update_secret(uint8_t* stored_sealed_activity_log)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    uint32_t enclave_ret = 0;
    sgx_ret = update_sealed_policy(enclave_id, &enclave_ret,
        (uint8_t *)stored_sealed_activity_log, sealed_activity_log_length);
    if (sgx_ret)
    {
        cerr<<"call update_sealed_policy fail, error code = 0x"<< hex<< sgx_ret
            <<endl;
        return sgx_ret;
    } 
    if (enclave_ret)
    {
        cerr<<"cannot update_sealed_policy, function return fail, error code ="
            "0x"<< hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}


uint32_t ReplayProtectedDRM::update_secret()
{
    return update_secret(sealed_activity_log); 
}

uint32_t ReplayProtectedDRM:: delete_secret(uint8_t* stored_sealed_activity_log)
{
    sgx_status_t sgx_ret = SGX_ERROR_UNEXPECTED;
    uint32_t enclave_ret = 0;
    sgx_ret = delete_sealed_policy(enclave_id, &enclave_ret,
        (uint8_t *)stored_sealed_activity_log, sealed_activity_log_length);
    if (sgx_ret)
    {
        cerr<<"call delete_sealed_policy fail, error code = 0x"<< hex<< sgx_ret 
            <<endl;
        return sgx_ret;
    } 
    if (enclave_ret)
    {
        cerr<<"cannot delete_sealed_policy, function return fail, error code ="
            "0x"<< hex<< enclave_ret <<endl;
        return enclave_ret;
    }
    return 0;
}


uint32_t ReplayProtectedDRM::delete_secret()
{
    return delete_secret(sealed_activity_log);
}
