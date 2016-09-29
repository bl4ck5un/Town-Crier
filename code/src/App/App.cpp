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
    sgx_status_t st;

    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::cin >> new_db;
    if (new_db == "y")
    {
        sqlite3_drop();
        std::cout << "TC.db cleaned" << std::endl;
    }

    sqlite3_init(&db);

    ret = initialize_enclave(ENCLAVE_FILENAME, &eid);

    if (ret != 0) {
        LL_CRITICAL("Failed to initialize the enclave");
        goto exit;
    }
    else {
        LL_NOTICE("enclave %lu created", eid);
    }

    st = register_exception_handlers(eid, &ret);
    if (st != SGX_SUCCESS || ret )
    {
        LL_CRITICAL("Failed to register exception handlers");
    }

/*
 *  We don't care about the attestation at the moment.
 *  Revisit after we have the official attestation service.
 */
//  remote_att_init(eid);

    monitor_loop(eid);

exit:
    LL_CRITICAL("Info: all enclave closed successfully.");
}
