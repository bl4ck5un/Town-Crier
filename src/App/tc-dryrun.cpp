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

#define LOGURU_IMPLEMENTATION 1

#include <stdint.h>

#include <iostream>
#include <vector>
#include <string>

#include "Enclave_u.h"

#include "Common/Constants.h"
#include "App/Converter.h"
#include "App/utils.h"
#include "App/request-parser.h"
#include "App/debug.h"
#include "App/monitor.h"
#include "App/EthRPC.h"

using std::vector;
using std::cerr;
using std::endl;
using std::to_string;

extern ethRPCClient *rpc_client;
jsonrpc::HttpClient *httpclient;

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " block_num" << endl;
    std::exit(-1);
  }

  loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
  loguru::init(argc, argv);

  sgx_enclave_id_t eid;
  sgx_status_t st;
  int ret;

  ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_INFO("enclave %lu created", eid);
  }

  blocknum_t blocknum = std::strtoul(argv[1], nullptr, 10);

  try {
    httpclient = new jsonrpc::HttpClient("http://localhost:8200");
    rpc_client = new ethRPCClient(*httpclient);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }

  string db_name = "tmp_db_" + to_string(blocknum);
  OdbDriver driver(db_name, true);
  std::atomic<bool> quit(false);

  Monitor monitor(&driver, eid, quit);
  monitor.dontSentTx();

  try {
    monitor._process_one_block(blocknum);
  }
  catch (const std::exception & e) {
    cerr << e.what() << endl;
  }

  catch (...) {
    cerr << "unknown" << endl;
  }

  return 0;
}
