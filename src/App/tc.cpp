//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
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

// SGX headers
#include <sgx_uae_service.h>

// system headers
#include <atomic>
#include <csignal>
#include <iostream>
#include <string>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>

// app headers
#include "App/Enclave_u.h"
#include "App/eth_rpc.h"
#include "App/status_rpc_server.h"
#include "App/attestation.h"
#include "App/bookkeeping/database.h"
#include "App/config.h"
#include "App/key_utils.h"
#include "App/monitor.h"
#include "App/request_parser.h"
#include "App/tc_exception.h"
#include "App/utils.h"
#include "App/logging.h"
#include "Common/Constants.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace tc {
namespace main {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("tc.cpp"));
}
}


extern ethRPCClient *geth_connector;

std::atomic<bool> quit(false);
void exitGraceful(int) { quit.store(true); }

int main(int argc, const char *argv[]) {
  using tc::main::logger;
  // parse command line arguments first
  tc::Config config(argc, argv);

  log4cxx::PropertyConfigurator::configure(LOGGING_CONF_FILE);

  // create working dir if not existed
  fs::create_directory(fs::path(config.getWorkingDir()));

  // logging to file
  fs::path log_path;
  char _log_tag[100] = {0};
  std::time_t _current_time = std::time(nullptr);
  if (std::strftime(_log_tag, sizeof _log_tag, "%F-%T",
                    std::localtime(&_current_time))) {
    log_path =
        fs::path(config.getWorkingDir()) / (string(_log_tag) + ".log");
  } else {
    log_path = fs::path(config.getWorkingDir()) / ("tc.log");
  }

  LL_INFO("config:\n%s", config.toString().c_str());

  try {
    auto httpclient = new jsonrpc::HttpClient(config.getGethRpcAddr());
    geth_connector = new ethRPCClient(*httpclient);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }

  int ret;
  sgx_enclave_id_t eid;
  sgx_status_t st;

  // init enclave first
  ret = initialize_enclave(config.getEnclavePath().c_str(), &eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_INFO("Enclave %ld created", eid);
  }

  // print MR and exit if requested
  if (config.printMR()) {
    cout << get_mr_enclave(eid) << endl;
    std::exit(0);
  }

  // register Ctrl-C handle
  std::signal(SIGINT, exitGraceful);
  // handle systemd termination signal
  std::signal(SIGTERM, exitGraceful);

  /*
   * set up database
   */
  static const string db_name = (fs::path(config.getWorkingDir()) / "tc.db").string();
  LL_INFO("using db %s", db_name.c_str());

  /*
  if (fs::exists(db_name)) {
    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::getline(std::cin, new_db);
    overwrite_old_db = new_db == "y";
  } else {
    overwrite_old_db = true;
  }
  LL_INFO("using new db: %d", overwrite_old_db);
  */

  // create if not exist
  bool overwrite_old_db = true;
  if (fs::exists(db_name)) {
    overwrite_old_db = false;
  }

  OdbDriver driver(db_name, overwrite_old_db);

  string wallet_address, hybrid_pubkey;

  try {
    wallet_address = unseal_key(eid, config.getSealedSigKey(), tc::keyUtils::ECDSA_KEY);
    hybrid_pubkey = unseal_key(eid, config.getSealedHybridKey(), tc::keyUtils::HYBRID_ENCRYPTION_KEY);

    LL_INFO("using wallet address at %s", wallet_address.c_str());
    LL_INFO("using hybrid pubkey: %s", hybrid_pubkey.c_str());

    provision_key(eid, config.getSealedSigKey(), tc::keyUtils::ECDSA_KEY);
    provision_key(eid, config.getSealedHybridKey(), tc::keyUtils::HYBRID_ENCRYPTION_KEY);
  } catch (const tc::EcallException &e) {
    LL_CRITICAL("%s", e.what());
    exit(-1);
  } catch (const std::exception& e) {
    LL_CRITICAL("%s", e.what());
    exit(-1);
  }

  jsonrpc::HttpServer status_server_connector(config.get_status_server_port(),
                                              "", "", 3);
  tc::status_rpc_server stat_srvr(status_server_connector, eid, driver);
  if (config.isStatusServerEnabled()) {
    stat_srvr.StartListening();
    LL_INFO("RPC server started at %d", config.get_status_server_port());
  }

  init(eid);
  set_env(eid, "a", "env");

  Monitor monitor(&driver, eid, quit);
  /* monitor.dontSendResponse(); */
  monitor.loop();

  if (config.isStatusServerEnabled()) {
    stat_srvr.StopListening();
  }
  sgx_destroy_enclave(eid);
  delete geth_connector;
  LL_INFO("all enclave closed successfully");
}
