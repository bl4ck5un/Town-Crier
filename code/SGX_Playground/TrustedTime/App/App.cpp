// App.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include "Enclave_u.h"
#include "sgx_error.h"
#include "sgx_urts.h"
#include <Windows.h>

#include "sgx_uae_service.h"

#define ENCLAVE_FILE _T("Enclave.signed.dll")
#define MAX_BUF_LEN 100

void ocall_print_string(const char* str) {
    printf("%s", str);
}

int main(int argc, char* argv[])
{
    sgx_enclave_id_t    eid;
    sgx_status_t        ret = SGX_SUCCESS;
    sgx_launch_token_t  token = {0};
    int updated = 0;

    ret = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, 
        &updated, &eid, NULL);

    if (ret != SGX_SUCCESS) {
        printf("App: Error %#x, failed to created enclave.\n", ret);
        return -1;
    }
    else {
        printf("Enclaved created. eid=%d\n", eid);
    }

    sgx_target_info_t target;
    sgx_epid_group_id_t gid;
    sgx_status_t st = sgx_init_quote (&target, &gid);
    printf("sgx_init_quote returned %d\n", st);


    create_session(eid);

    printf("Start querying trusted time\n");
    for (int i = 0; i < 32; i++) {
        ret = showtime(eid);
        if (ret != SGX_SUCCESS) {
            printf("App: Error %#x, failed to ecall showtime().\n", ret);
            return -1;
        }
        Sleep(3000);
    }

#if defined(WIN32)
    printf("Press Enter to exit the program. ");
    getchar();
#endif
    return 0;
}

