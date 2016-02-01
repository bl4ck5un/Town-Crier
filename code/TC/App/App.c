#include "App.h"

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "App.h"
#include "RemoteAtt.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

//#define RPC_TEST
#define ECDSA_TEST
//#define SCRAPER_TEST

int main()
{
    int ret;

#ifdef RPC_TEST
    test_rpc();
#endif

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }
#endif 
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }
 
#if defined(SCRAPER_TEST)
    test_yahoo_finance(global_eid, &ret);
    if (ret != 0) {
        printf("test_yahoo_finance returned %d\n", ret);
    }
#endif

#if defined(ECDSA_TEST)
    test_ecdsa(global_eid, &ret);
    if (ret != 0) {
        printf("test_yahoo_finance returned %d\n", ret);
    }
#endif

    remote_att_init();

    printf("Info: SampleEnclave successfully returned.\n");
    printf("Enter a character before exit ...\n");
    getchar();
    return 0;
}
