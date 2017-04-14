#include <iostream>
#include <stdio.h>
#include "ExampleEnclave_u.h"

#include "Utils.h"

#include "sgx_urts.h"

using namespace std;

/* Global EID shared by multiple threads */
sgx_enclave_id_t eid = 0;


int server()
{
    int ret;
    sgx_accept(eid, &ret);
    return ret;
}

int client()
{
    int ret = 0;
    sgx_connect(eid, &ret);
    return ret;
}

int main()
{
    int ret;

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        cout << "sgx is not support" << endl;
        ret = -1; 
        goto exit;
    }
#endif

    ret = initialize_enclave(&eid);

/* 
    // a simpler way to initialize an enclave
    int updated = 0;
    sgx_launch_token_t token = {0};
    sgx_status_t st;

    st = sgx_create_enclavea(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
    if (st != SGX_SUCCESS)
    {
        print_error_message(st);
        printf("failed to create enclave. Returned %#x", st);
        goto exit;
    }
*/
    if (ret != 0)
    {
        goto exit;
    }
    printf("Enclave %llu created\n", eid);
    client();

exit:
    printf("%%Info: all enclave closed successfully.\n");
    printf("%%Enter a character before exit ...\n");
    system("pause");
    return 0;
}