#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _MSC_VER
# include <Shlobj.h>
#else
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX
#endif

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "App.h"
#include "Enclave_u.h"


int test_connect()
{
    int ret;
    sgx_status_t st = ecall_client(global_eid, &ret, "google.com", "443");

    if (st != SGX_SUCCESS) {
        printf("connect failed!\n");
    }

    return ret;

}

int test_self_test()
{
    int ret;
    sgx_status_t st = ecall_self_test(global_eid, &ret);
    
    if (st != SGX_SUCCESS) {
        printf("connect failed!\n");
    }

    return ret;
}

