#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <csignal>

#include "sgx_urts.h"
#include "sgx_uae_service.h"

#include "Enclave_u.h"
#include "attestation.h"
#include "stdint.h"
#include "EthRPC.h"
#include "request-parser.hxx"
#include "utils.h"
#include "Log.h"
#include "Monitor.h"
#include "utils.h"
#include "Constants.h"
#include "StatusRpcServer.h"
#include "bookkeeping/database.hxx"


namespace po = boost::program_options;

extern ethRPCClient *rpc_client;
jsonrpc::HttpClient *httpclient;
boost::property_tree::ptree pt;

std::atomic<bool> quit(false);

void exitGraceful(int) {
  quit.store(true);
}

void init(const string filepath) {
  try {
    boost::property_tree::ini_parser::read_ini(filepath, pt);
    std::string st = pt.get<std::string>("RPC.RPChost");

    std::cout << st << std::endl;
    httpclient = new jsonrpc::HttpClient(st);
    rpc_client = new ethRPCClient(*httpclient);

  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }
}

int main(int argc, const char *argv[]) {
  bool options_rpc = false;
  string options_config = "config";

  try {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "print this message")
        ("rpc", po::bool_switch(&options_rpc)->default_value(false), "Launch RPC server")
        ("config,c", po::value(&options_config)->default_value("config"), "Path to a config file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cerr << desc << endl;
      return -1;
    }

    po::notify(vm);
  }

  catch (po::required_option &e) {
    cerr << e.what() << endl;
    return -1;
  }
  catch (std::exception &e) {
    cerr << e.what() << endl;
    return -1;
  }
  catch (...) {
    cerr << "Unknown error!" << endl;
    return -1;
  }

  init(options_config);

  int ret;
  sgx_enclave_id_t eid;
  sgx_status_t st;

  //! register Ctrl-C handler
  std::signal(SIGINT, exitGraceful);

  LL_NOTICE("Setting up RPC Server");
  jsonrpc::HttpServer httpServer(8123);
  StatusRpcServer statusRpcServer(httpServer, eid);
  if (options_rpc) {
    statusRpcServer.StartListening();
  }

  const static string db_name = "TC.db";
  bool create_db = false;
  if (boost::filesystem::exists(db_name)) {
    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::getline(std::cin, new_db);
    create_db = new_db == "y";
  } else {
    create_db = true;
  }
  OdbDriver driver("TC.db", create_db);

  int nonce_offset = 0;

  ret = initialize_tc_enclave(&eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_NOTICE("enclave %lu created", eid);
  }

  Monitor monitor(driver, eid, nonce_offset, quit);
  monitor.loop();

  if (options_rpc) {
    statusRpcServer.StopListening();
  }
  sgx_destroy_enclave(eid);
  LL_CRITICAL("Info: all enclave closed successfully.");
}
