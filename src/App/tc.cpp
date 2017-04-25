//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TowCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//


// system headers
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <stdint.h>

// SGX headers
#include <sgx_uae_service.h>

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

#include "Common/Constants.h"
#include "App/Enclave_u.h"
#include "App/EthRPC.h"
#include "App/StatRPCServer.h"
#include "App/attestation.h"
#include "App/bookkeeping/database.h"
#include "App/key-utils.h"
#include "App/monitor.h"
#include "App/request-parser.h"
#include "App/tc-exception.h"
#include "App/utils.h"

#define LOGURU_IMPLEMENTATION 1
#include "Common/Log.h"
#include "App/config.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

extern ethRPCClient *rpc_client;
jsonrpc::HttpClient *httpclient;

std::atomic<bool> quit(false);
void exitGraceful(int) { quit.store(true); }

int main(int argc, const char *argv[]) {
  // init logging
  loguru::init(argc, argv);

  tc::Config config(argc, argv);
  cout << config.to_string();

  // create working dir if not existed
  fs::create_directory(fs::path(config.get_working_dir()));

  // logging to file
  fs::path log_path;
  char _log_tag[100] = {0};
  std::time_t _current_time = std::time(NULL);
  if (std::strftime(_log_tag, sizeof _log_tag, "%F-%T",
                    std::localtime(&_current_time))) {
    log_path =
        fs::path(config.get_working_dir()) / ("tc" + string(_log_tag) + ".log");
  } else {
    log_path = fs::path(config.get_working_dir()) / ("tc.log");
  }
  loguru::add_file(log_path.c_str(), loguru::Append, loguru::Verbosity_MAX);

  LL_INFO("config:\n%s", config.to_string().c_str());

  try {
    httpclient = new jsonrpc::HttpClient(config.get_geth_rpc_addr());
    rpc_client = new ethRPCClient(*httpclient);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }

  int ret;
  sgx_enclave_id_t eid;
  sgx_status_t st;

  // register Ctrl-C handler
  std::signal(SIGINT, exitGraceful);
  // handle systemd termination signal
  std::signal(SIGTERM, exitGraceful);

  if (config.is_run_as_daemon()) {
#ifdef CONFIG_IMPL_DAEMON
    daemonize(opt_cwd, pid_filename);
#else
    LL_CRITICAL("*** daemonize() is not implemented ***");
#endif
  }

  static const string db_name =
      (fs::path(config.get_working_dir()) / "tc.db").string();
  LOG_F(INFO, "using db %s", db_name.c_str());
  bool create_db = false;
  if (fs::exists(db_name) && !config.is_run_as_daemon()) {
    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::getline(std::cin, new_db);
    create_db = new_db == "y";
  } else {
    create_db = true;
  }
  LL_INFO("using new db: %d", create_db);
  OdbDriver driver(db_name, create_db);

  int nonce_offset = 0;

  ret = initialize_enclave(config.get_enclave_path().c_str(), &eid);
  if (ret != 0) {
    LOG_F(FATAL, "Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LOG_F(INFO, "Enclave %ld created", eid);
  }

  string address;

  try {
    address = unseal_key(eid, config.get_sealed_sig_key());
    LL_INFO("using address %s", address.c_str());

    provision_key(eid, config.get_sealed_sig_key());
  } catch (const tc::EcallException &e) {
    LL_CRITICAL("%s", e.what());
    exit(-1);
  }

  jsonrpc::HttpServer status_server_connector(config.get_status_server_port(),
                                              "", "", 3);
  tc::StatRPCServer stat_srvr(status_server_connector, eid, driver);
  if (config.is_status_server_enabled()) {
    stat_srvr.StartListening();
    LOG_F(INFO, "RPC server started");
  }

  Monitor monitor(&driver, eid, nonce_offset, quit);
  monitor.loop();

  if (config.is_status_server_enabled()) {
    stat_srvr.StopListening();
  }
  sgx_destroy_enclave(eid);
  delete rpc_client;
  delete httpclient;
  LOG_F(INFO, "all enclave closed successfully");
}
