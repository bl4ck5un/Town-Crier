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
#include "App/eth_rpc.h"
#include "App/status_rpc_server.h"
#include "App/attestation.h"
#include "App/bookkeeping/database.h"
#include "App/key_utils.h"
#include "App/monitor.h"
#include "App/request_parser.h"
#include "App/tc_exception.h"
#include "App/utils.h"

#define LOGURU_IMPLEMENTATION 1
#include "Common/Log.h"
#include "App/config.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

extern ethRPCClient *geth_connector;

std::atomic<bool> quit(false);
void exitGraceful(int) { quit.store(true); }

int main(int argc, const char *argv[]) {
  // init logging
  loguru::init(argc, argv);

  tc::Config config(argc, argv);

  // create working dir if not existed
  fs::create_directory(fs::path(config.getWorkingDir()));

  // logging to file
  fs::path log_path;
  char _log_tag[100] = {0};
  std::time_t _current_time = std::time(NULL);
  if (std::strftime(_log_tag, sizeof _log_tag, "%F-%T",
                    std::localtime(&_current_time))) {
    log_path =
        fs::path(config.getWorkingDir()) / ("tc" + string(_log_tag) + ".log");
  } else {
    log_path = fs::path(config.getWorkingDir()) / ("tc.log");
  }
  loguru::add_file(log_path.c_str(), loguru::Append, loguru::Verbosity_MAX);

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
    LOG_F(FATAL, "Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LOG_F(INFO, "Enclave %ld created", eid);
  }

  // print MR and exit if requested
  if (config.printMR()) {
    cout << get_mr_enclave(eid);
    std::exit(0);
  }

  // register Ctrl-C handle
  std::signal(SIGINT, exitGraceful);
  // handle systemd termination signal
  std::signal(SIGTERM, exitGraceful);

  if (config.isRunAsDaemon()) {
#ifdef CONFIG_IMPL_DAEMON
    daemonize(opt_cwd, pid_filename);
#else
    LL_CRITICAL("*** daemonize() is not implemented ***");
#endif
  }

  /*
   * set up database
   */
  static const string db_name = (fs::path(config.getWorkingDir()) / "tc.db").string();
  LOG_F(INFO, "using db %s", db_name.c_str());
  bool overwrite_old_db = false;
  if (fs::exists(db_name) && !config.isRunAsDaemon()) {
    std::cout << "Do you want to clean up the database? y/[n] ";
    std::string new_db;
    std::getline(std::cin, new_db);
    overwrite_old_db = new_db == "y";
  } else {
    overwrite_old_db = true;
  }
  LL_INFO("using new db: %d", overwrite_old_db);
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
  }

  jsonrpc::HttpServer status_server_connector(config.get_status_server_port(),
                                              "", "", 3);
  tc::status_rpc_server stat_srvr(status_server_connector, eid, driver);
  if (config.isStatusServerEnabled()) {
    stat_srvr.StartListening();
    LOG_F(INFO, "RPC server started");
  }

  init(eid);
  set_env(eid, "a", "env");

  Monitor monitor(&driver, eid, quit);
//  monitor.dontSendResponse();
  monitor.loop();

  if (config.isStatusServerEnabled()) {
    stat_srvr.StopListening();
  }
  sgx_destroy_enclave(eid);
  delete geth_connector;
  LOG_F(INFO, "all enclave closed successfully");
}
