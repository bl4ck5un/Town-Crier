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

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <mutex>
#include <thread>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <iostream>

sgx_enclave_id_t global_eid = 0;
sqlite3* db = NULL;

#define SHORTCUT_GETH

int main()
{
    int ret;

#if !defined(SHORTCUT_GETH)
    std::cout << "Clean up database? y/[n] ";
    std::string new_db;
    std::cin >> new_db;
    if (new_db == "y")
    {
        sqlite3_drop();
        std::cout << "TC.db cleaned" << std::endl;
    }

    sqlite3_init(&db);
#endif

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        LL_CRITICAL("sgx is not support");
        ret = -1; 
        goto error;
    }
#endif 

    int updated = 0;
    sgx_launch_token_t token = {0};
    sgx_status_t st;

    st = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (st != SGX_SUCCESS)
    {
        print_error_message(st);
        LL_CRITICAL("Failed to create enclave. Returned %#x", st);
		TCHAR pwd[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, pwd);
		LL_CRITICAL("pwd is %s", pwd);
		goto error;
    }

    LL_NOTICE("enclave %llu created", global_eid);

#if !defined(SHORTCUT_GETH)
    monitor_loop(global_eid);
#endif

#if defined(SHORTCUT_GETH)
	int retval = 0;
    char res[32];
	ups_tracking(global_eid, &retval, "123", res);
	LL_LOG("ups_tracking(..) returns %d", retval);
#endif

exit:
    LL_CRITICAL("Info: all enclave closed successfully.");
error:
    LL_CRITICAL("Enter a character before exit ...");
    system("pause");
 }