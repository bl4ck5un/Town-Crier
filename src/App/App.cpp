/* App.cpp is main function that is in charge of initializing the 
   Enclave and calling the main monitor loop
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <syslog.h>
#include <cfgparser.h>


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
#include "Monitor.h"
#include "Utils.h"
#include "Constants.h"

#define DAEMON_NAME "TownCrierDaemon"



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

int main(int argc, char* argv[]){
    //Set Logging Mask and open log
    init(argc, argv);
    int ret;
    sgx_enclave_id_t eid;
    sgx_status_t st;
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

    syslog(LOG_INFO, "Entering Town Crier Daemon");


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
        syslog(LOG_CRIT,"Failed to initialize the enclave");
        exit(EXIT_FAILURE);
    }
    else{
        syslog(LOG_INFO, "enclave %lu created", eid);
    }

    st = register_exception_handlers(eid, &ret);
    if (st != SGX_SUCCESS || ret )
    {
        syslog(LOG_INFO, "Failed to register exception handlers");
    }

/*
 *  We don't care about the attestation at the moment.
 *  Revisit after we have the official attestation service.
 */
//  remote_att_init(eid);

    pid_t pid, sid;
    pid = fork();
    //Failed to initialize for some reason, exit
    if(pid < 0){
        syslog(LOG_CRIT, "Fork Failed");
        exit(EXIT_FAILURE);
    } 

    if(pid > 0){
        exit(EXIT_SUCCESS);
    }

    sid = setsid();
    if(sid < 0) { 
        syslog(LOG_CRIT, "setsid Failed");
        exit(EXIT_FAILURE);
    }

    //Since this is a Daemon, close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //Begin the main process
    monitor_loop(eid, nonce);

    closelog();

exit:
    syslog(LOG_CRIT, "all enclave closed successfully.");
}
