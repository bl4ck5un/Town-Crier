#include "App.h"

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "RemoteAtt.h"
#include "stdint.h"
#include "EthRPC.h"
#include "sqlite3.h"
#include "Bookkeeping.h"

#include "WinBase.h"
#include "Log.h"
#include "Monitor.h"
#include "Utils.h"
#include "Constants.h"

#include <sstream>
#include <string>
#include <ctime>

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

#define RPC_TEST
//#define ECDSA_TEST
//#define SCRAPER_TEST
//#define REMOTE_ATT_TEST

//#define ENCLAVE_TEST


#define SEALED_NONCE_SIZE 0x250

#ifdef ENCLAVE_TEST
int main()
{
    int ret;
    sgx_status_t st;
    if(initialize_enclave() < 0){
        LL_CRITICAL("Failed to init the enclave");
        goto blocked_exit;
    }
    st = Test_main(global_eid, &ret);
    if (st != SGX_SUCCESS)
    {
        LL_CRITICAL("Failed to launch test");
        print_error_message(st);
    }
    LL_NOTICE("Info: SampleEnclave successfully returned %d", ret);
    
blocked_exit:
    LL_NOTICE("Enter a character before exit ...");
    getchar();
    return 0;
}
//#ifdef TEST
//int main()
//{
//    int ret, tx_len;
//    uint8_t tx[2048];
//    char* tx_str;

//    uint8_t nonce[32];

//    memset(nonce, 0, 32);

//#if defined(_MSC_VER)
//    if (query_sgx_status() < 0) {
//        LL_CRITICAL("sgx is not support");
//        ret = -1; goto exit;
//    }
//#endif 
//    if(initialize_enclave() < 0){
//        printf("Enter a character before exit ...\n");
//        getchar();
//        return -1; 
//    }

//#ifdef RPC_TEST
//    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(NONCE_FILE_NAME) &&
//        GetLastError() == ERROR_FILE_NOT_FOUND)
//        { dump_nonce(nonce); }
//    else
//        { load_nonce(nonce); }

//    dump_buf("nonce to be used: ", nonce, 32);
//    get_raw_signed_tx(global_eid, &ret, nonce, 32, tx, &tx_len);

//    if (ret != 0)
//    {
//        fprintf(stderr, "get_raw_signed_tx returned %d\n", ret);
//        goto exit;
//    }
//    tx_str = (char*) malloc(tx_len * 2 + 1);
//    char2hex(tx, tx_len, tx_str);
//#ifdef VERBOSE
//    dump_buf("tx body: ", tx, tx_len);
//    printf("tx_str: %s\n", tx_str);
//#endif
//    ret = send_transaction(tx_str);
//    if (ret != 0)
//    {
//        fprintf(stderr, "send_raw_tx returned %d\n", ret);
//        goto exit;        
//    }
//    else
//    {
//        dump_buf("new nonce being dumped: ", nonce, 32);
//        dump_nonce(nonce);
//    }
//#endif
 
//#if defined(SCRAPER_TEST)
//    scraper_dispatch(global_eid, &ret);
//    if (ret != 0) {
//        LL_CRITICAL("test_yahoo_finance returned %d", ret);
//        goto exit;
//    }
//    LL_NOTICE("test\n");
//#endif

//#if defined(ECDSA_TEST)
//    test_ecdsa(global_eid, &ret);
//    if (ret != 0) {
//        printf("test_ecdsa returned %d", ret);
//    }
//#endif

//#ifdef REMOTE_ATT_TEST
//    remote_att_init();  
//#endif // REMOTE_ATT_TEST

//exit:
//     test_RLP(global_eid, &ret);
//    printf("Info: SampleEnclave successfully returned.\n");
//    printf("Enter a character before exit ...\n");
//    getchar();
//    return 0;
//}
#else
int main()
{
    int ret, tx_len;
    uint8_t tx[2048];
    char* tx_str;

    uint8_t nonce[32] = {0};

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(NONCE_FILE_NAME) &&
        GetLastError() == ERROR_FILE_NOT_FOUND)
        { dump_nonce(nonce); }
    else
        { load_nonce(nonce); }

//    sqlite3* db;
//    ret = sqlite3_init(&db);
//    if (ret)
//    {
//        LL_CRITICAL("Error opening SQLite3 database: %s", sqlite3_errmsg(db));
//        sqlite3_close(db);
//    }
//
//    ret = record_nonce(db, 15);
//    if (ret)
//    {
//        goto exit;
//    }

//    int nonces;
//    ret = get_last_nonce(db, &nonces);
//    if (ret) goto exit;
//    std::cout << "nonce: " << nonces << std::endl;


//    dump_buf("nonce to be used: ", nonce, 32);

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        LL_CRITICAL("sgx is not support");
        ret = -1; goto exit;
    }
#endif 
    if(initialize_enclave() < 0){
        LL_CRITICAL("Enter a character before exit ...\n");
        ret = -1; goto exit;
    }

    // main loop
#ifdef E2E_BENCHMARK
//    for (int i = 0; i < 5; i++)
        monitor_loop(nonce);
#endif
exit:
    // test_RLP(global_eid, &ret);
//    sqlite3_close(db);
    LL_NOTICE("Info: SampleEnclave successfully returned.\n");
    LL_NOTICE("Enter a character before exit ...\n");
    getchar();
    return 0;
}
#endif

