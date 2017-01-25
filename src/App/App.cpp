#include "App.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include "sgx_urts.h"
#include "sgx_uae_service.h"
#include "Enclave_u.h"
#include "RemoteAtt.h"
#include "stdint.h"
#include "EthRPC.h"
#include "request-parser.hxx"
#include "sqlite3.h"
#include "Bookkeeping.h"
#include "Init.h"

#include "Log.h"
#include "Monitor.h"
#include "Utils.h"
#include "Constants.h"

#include "StatusRpcServer.h"
#include "bookkeeping/database.hxx"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <csignal>

#include <boost/filesystem.hpp>

sqlite3 *db = NULL;

extern ethRPCClient *c;
jsonrpc::HttpClient *httpclient;
boost::property_tree::ptree pt;

std::atomic<bool> quit(false);

void exitGraceful(int) {
  quit.store(true);
}

void init(int argc, char *argv[]) {
  std::string filepath;
  if (argc < 2) {
    filepath = "config";
  } else {
    filepath = argv[1];
  }

  try {
    boost::property_tree::ini_parser::read_ini(filepath, pt);
    std::string st = pt.get<std::string>("RPC.RPChost");

    std::cout << st << std::endl;
    httpclient = new jsonrpc::HttpClient(st);
    c = new ethRPCClient(*httpclient);

  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }
}

int main(int argc, char *argv[]) {
  init(argc, argv);
  int ret;
  sgx_enclave_id_t eid;
  sgx_status_t st;

  //! register Ctrl-C handler
  std::signal(SIGINT, exitGraceful);


  jsonrpc::HttpServer httpServer(8123);
  StatusRpcServer statusRpcServer(httpServer);
  statusRpcServer.StartListening();

  const static string db_name = "TC.db";
  bool create_db = false;
  if (boost::filesystem::exists(db_name)) {
    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::cin >> new_db;
    create_db = new_db == "y";
  } else {
    create_db = true;
  }
  OdbDriver driver("TC.db", create_db);

  int nonce_offset = 0;
//  if (argc == 2) {
//    nonce = atoi(argv[1]);
//  }

//  if (nonce > 0)
//    dump_nonce((uint8_t *) &nonce);

  ret = initialize_tc_enclave(&eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_NOTICE("enclave %lu created", eid);
  }

/*
 *  We don't care about the attestation at the moment.
 *  Revisit after we have the official attestation service.
 */
//  remote_att_init(eid);

  Monitor monitor(driver, eid, nonce_offset, quit);
  monitor.loop();

  sgx_destroy_enclave(eid);
  LL_CRITICAL("Info: all enclave closed successfully.");
}
