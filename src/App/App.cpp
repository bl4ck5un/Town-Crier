#include "App.h"

#include "cfgparser.h"

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
#include <fstream>

sqlite3* db = NULL;

extern ethRPCClient *c;

jsonrpc::HttpClient *httpclient;

ConfigParser_t cfg;

void init(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "please specify the path to the configuration" << std::endl;
        exit(-1);
    }

    std::string st = string("localhost");
    /*
    if (cfg.readFile(argv[1]))
    {
        std::cout << "Error: Cannot open config file " << argv[1] << std::endl;
        exit(-1);
    }
    if (!cfg.getValue("RPC", "RPChost", &st)) {
        std::cout << "Error: Cannot open RPC host!" << std::endl;
        exit(-1);
    }
    */

    std::cout << st << std::endl;
    httpclient = new jsonrpc::HttpClient(st);
    c = new ethRPCClient(*httpclient);
}

int main(int argc, char* argv[])
{
    init(argc, argv);
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

    int nonce = 0;
    if (argc == 2) {
        nonce = atoi(argv[1]);
    }

    if (nonce > 0)
        dump_nonce((uint8_t*)&nonce);

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

    monitor_loop(eid, nonce);

exit:
    LL_CRITICAL("Info: all enclave closed successfully.");
}
