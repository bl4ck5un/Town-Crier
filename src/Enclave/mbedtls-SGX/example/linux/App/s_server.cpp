#include <iostream>
#include <stdio.h>
#include <sgx_urts.h>

#include "Enclave_u.h"
#include "Utils.h"


using namespace std;

int main()
{
    sgx_enclave_id_t eid = 0;
    int ret;

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        cout << "sgx is not support" << endl;
        ret = -1; 
        goto exit;
    }
#endif

    ret = initialize_enclave(&eid);
    if (ret != 0)
    {
        goto exit;
    }
    printf("Enclave %lu created\n", eid);

    sgx_accept(eid, &ret);
exit:
    printf("Info: all enclave closed successfully.\n");
    printf("Enter a character before exit ...\n");
    return 0;
}
