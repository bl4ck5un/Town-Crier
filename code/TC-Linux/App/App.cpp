#include "App.h"

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "RemoteAtt.h"
#include "stdint.h"
#include "EthRPC.h"
#include "sqlite3.h"
#include "Bookkeeping.h"

#ifdef _WIN32
#include "WinBase.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#endif 
#include "Log.h"
#include "Monitor.h"
#include "Utils.h"
#include "Constants.h"

#ifdef _WIN32
#include <mutex>
#include <thread>
#endif

#include <stdio.h>
#include <iostream>


/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
sqlite3* db = NULL;

#ifdef _WIN32

#ifdef E2E_BENCHMARK_THREADING
#define N_ENCLAVE 19
#else
#define N_ENCLAVE 1
#endif
#define N_REQUEST 100
sgx_enclave_id_t eids[N_ENCLAVE];
std::mutex locks[N_ENCLAVE];

typedef struct _benchmark_params_t {
    int id;
    uint8_t type;
} benchmark_params_t;

#ifdef E2E_BENCHMARK_THREADING
VOID CALLBACK handling_thread(
    PTP_CALLBACK_INSTANCE Instance, PVOID param, PTP_WORK Work){
    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(Work);
    assert(param != NULL);
    benchmark_params_t* params = (benchmark_params_t*) param;
    int id = params->id;
    uint8_t type = params->type;
#else
void handling_thread(int id, int type){
#endif
    assert(id >= 0 && id <= N_ENCLAVE);
//    printf("+%d", id);
    locks[id].lock();
//    printf("-%d", id);
    int nonce = 0;
//    remote_att_init();
    int ret = 0;
    char req[64] = {0};
    req[0] = 'G';
    req[1] = 'O';
    req[2] = 'O';
    req[3] = 'G';
    req[4] = 'L';
    uint8_t raw_tx[TX_BUF_SIZE];
    int raw_tx_len = sizeof raw_tx;
    long long unsigned time1 = 0 , time2 = 0;
#ifdef E2E_BENCHMARK
    time1 = __rdtsc();
    LL_CRITICAL("switch in begins: %llu", time1);
#endif
    ret = handle_request(eids[id], &ret, nonce, 0xFF, type, 
        (uint8_t*) req, sizeof req, raw_tx, &raw_tx_len);
#ifdef E2E_BENCHMARK
    time2 = __rdtsc();
    LL_CRITICAL("switch out done: %llu", time2);
    LL_CRITICAL("handle_request overalll: %llu", time2-time1);
#endif
    if (ret != 0)
    {
        LL_CRITICAL("handle_request returned %d", ret);
        locks[id].unlock();
        return;
    }
    locks[id].unlock();
    return;
}

int test_main()
{
    int ret, tx_len;
    uint8_t tx[2048];
    char* tx_str;

    uint8_t nonce[32] = {0};
    std::vector<std::thread> threads;

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(NONCE_FILE_NAME) &&
        GetLastError() == ERROR_FILE_NOT_FOUND)
        { dump_nonce(nonce); }
    else
        { load_nonce(nonce); }

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        LL_CRITICAL("sgx is not support");
        ret = -1; goto exit;
    }
#endif 

    int updated = 0;
    sgx_launch_token_t token = {0};
    sgx_status_t st;




#ifdef BENCHMARK
    long long time1 = 0, time2 = 0;
#endif
#ifdef OFFLINE_BENCHMARK
    for (int j = 0; j < 100; j++)
    {
        time1 = __rdtsc();
        int i = 0;
        st = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &eids[i], NULL);
        if (st != SGX_SUCCESS)
        {
            print_error_message(st);
            LL_CRITICAL("%d: failed to create enclave. Returned %#x", i, st);
        }
        LL_NOTICE("%d: enclave %llu created", i, eids[i]);
        time2 = __rdtsc();
        LL_CRITICAL("setup time: %f", (time2-time1)/FREQ);       
        st = sgx_destroy_enclave(eids[i]);
        if (st != SGX_SUCCESS)
        {
            print_error_message(st);
            LL_CRITICAL("%d: failed to create enclave. Returned %#x", i, st);
        }
    }
    goto exit;

#endif
#ifdef TIME_CALIBRATION_BENCHMARK
    for (int j = 0; j < 100; j++)
    {
        time1 = __rdtsc();
        remote_att_init(eids[0]);
        time2 = __rdtsc();
        LL_CRITICAL("remote_att_init overall: %f", (time2-time1)/FREQ);
        time_calibrate(eids[0]);
        time1 = __rdtsc();
        LL_CRITICAL("time_calibrate overall: %f", (time1-time2)/FREQ);
    }

    goto exit;
#endif



#ifdef E2E_BENCHMARK
    for (int i = 0; i < 500; i++)
    {
        handling_thread(0, TYPE_FINANCE_INFO);
        printf("%%\n");
    }
#endif

#ifdef E2E_BENCHMARK_THREADING
    for (int i = 0; i < N_ENCLAVE; i++)
    {
            st = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &eids[i], NULL);
            if (st != SGX_SUCCESS)
            {
                print_error_message(st);
                LL_CRITICAL("%d: failed to create enclave. Returned %#x", i, st);
            }
            LL_NOTICE("%d: enclave %llu created", i, eids[i]);
    }
    BOOL bRet = FALSE;
    PTP_WORK work = NULL;
    PTP_TIMER timer = NULL;
    PTP_POOL pool = NULL;
    PTP_WORK_CALLBACK workcallback = handling_thread;
    TP_CALLBACK_ENVIRON CallBackEnviron;
    PTP_CLEANUP_GROUP cleanupgroup = NULL;
    FILETIME FileDueTime;
    ULARGE_INTEGER ulDueTime;
    UINT rollback = 0;

    InitializeThreadpoolEnvironment(&CallBackEnviron);
    pool = CreateThreadpool(NULL);
    if (NULL == pool) {
        _tprintf(_T("CreateThreadpool failed. LastError: %u\n"),
                     GetLastError());
        goto main_cleanup;
    }

    rollback = 1; // pool creation succeeded
    SetThreadpoolThreadMaximum(pool, 20);
    bRet = SetThreadpoolThreadMinimum(pool, 20);
    if (FALSE == bRet) {
        _tprintf(_T("SetThreadpoolThreadMinimum failed. LastError: %u\n"),
                     GetLastError());
        goto main_cleanup;
    }

    cleanupgroup = CreateThreadpoolCleanupGroup();
    if (NULL == cleanupgroup) {
        _tprintf(_T("CreateThreadpoolCleanupGroup failed. LastError: %u\n"), 
                     GetLastError());
        goto main_cleanup; 
    }

    rollback = 2;  // Cleanup group creation succeeded

    SetThreadpoolCallbackPool(&CallBackEnviron, pool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron,
                                      cleanupgroup,
                                      NULL);

    int n_enclave[] = {2, 5, 10, 15, 19};
    benchmark_params_t params[N_REQUEST];


    for (int j = 0; j < 20; j++)
    {
        
    LL_CRITICAL("%%%d\n", j);

    for (auto n : n_enclave)
    {
        time1 = __rdtsc();
        for (int i = 0; i < N_REQUEST; i++)
        {
            params[i].id = i % n;
            params[i].type = (uint8_t) TYPE_FLIGHT_INS;
            work = CreateThreadpoolWork(workcallback, &params[i], &CallBackEnviron);
            if (NULL == work) {
                _tprintf(_T("CreateThreadpoolWork failed. LastError: %u\n"),
                     GetLastError());
                goto main_cleanup;
            }
            SubmitThreadpoolWork(work);
        }
        
        CloseThreadpoolCleanupGroupMembers(cleanupgroup, FALSE, NULL);
        time2 = __rdtsc();
        LL_CRITICAL("%d: %llu", n, time2 - time1); 
    }

    LL_CRITICAL("%%");

    }

    rollback = 3;  // Creation of work succeeded
    goto main_cleanup;

main_cleanup:
    switch (rollback) {
        case 4:
        case 3:
            // Clean up the cleanup group members.
            CloseThreadpoolCleanupGroupMembers(cleanupgroup,
                FALSE, NULL);
        case 2:
            // Clean up the cleanup group.
            CloseThreadpoolCleanupGroup(cleanupgroup);

        case 1:
            // Clean up the pool.
            CloseThreadpool(pool);

        default:
            break;
    }

    for (int i = 0; i < N_ENCLAVE; i++)
    {
            st = sgx_destroy_enclave(eids[i]);
            if (st != SGX_SUCCESS)
            {
                print_error_message(st);
                LL_CRITICAL("failed to close enclave. Returned %#x", st);
            }
    } 
#endif
exit:
    LL_CRITICAL("%%Info: all enclave closed successfully.");
    LL_CRITICAL("%%Enter a character before exit ...");
    getchar();
    return 0;
}
#endif

int main()
{
    int ret;

    std::cout << "Clean up database? y/[n] ";
    std::string new_db;
    std::cin >> new_db;
    if (new_db == "y")
    {
        sqlite3_drop();
        std::cout << "TC.db cleaned" << std::endl;
    }

    sqlite3_init(&db);

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        LL_CRITICAL("sgx is not support");
        ret = -1; 
        goto exit;
    }
#endif 

    int updated = 0;
    sgx_launch_token_t token = {0};
    sgx_status_t st;

    st = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (st != SGX_SUCCESS)
    {
        print_error_message(st);
        LL_CRITICAL("failed to create enclave. Returned %#x", st);
    }
    LL_NOTICE("enclave %llu created", global_eid);

//    remote_att_init(global_eid);
//    goto exit;

    monitor_loop(global_eid);

exit:
    LL_CRITICAL("Info: all enclave closed successfully.");
    LL_CRITICAL("Enter a character before exit ...");
    system("pause");
}
