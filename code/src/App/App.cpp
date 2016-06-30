#include "App.h"

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "RemoteAtt.h"
#include "stdint.h"
#include "EthRPC.h"
#include "sqlite3.h"
#include "Bookkeeping.h"
#include "Init.h"

#include "Log.h"
#include "Monitor.h"
#include "Utils.h"
#include "Constants.h"

#include <stdio.h>
#include <iostream>


sqlite3* db = NULL;

int main()
{
    int ret;
    sgx_enclave_id_t eid;

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

    ret = initialize_enclave(ENCLAVE_FILENAME, &eid);

    if (ret != 0) {
        goto exit;
    }
    else {
        LL_NOTICE("enclave %lu created", eid);
    }

    ssl_test(eid, &ret);
    return 0;
/*
 *  Uncomment to test attestation
 */
//  remote_att_init(eid);

    monitor_loop(eid);

exit:
    LL_CRITICAL("Info: all enclave closed successfully.");
}
