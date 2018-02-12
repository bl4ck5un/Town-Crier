/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially
 * docketed at the Center for Technology Licensing at Cornell University
 * as D-7364, developed through research conducted at Cornell University,
 * and its associated copyrights solely for educational, research and
 * non-profit purposes without fee is hereby granted, provided that the
 * user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling
 * the TownCrier source code. No other rights to use TownCrier and its
 * associated copyrights for any other purpose are granted herein,
 * whether commercial or non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial
 * products or use TownCrier and its associated copyrights for commercial
 * purposes must contact the Center for Technology Licensing at Cornell
 * University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
 * ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
 * commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
 * ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
 * UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
 * REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
 * OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
 * OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
 * PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science
 * Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
 * CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
 * Google Faculty Research Awards, and a VMWare Research Award.
 */

#ifndef SRC_APP_MONITOR_H_
#define SRC_APP_MONITOR_H_

#include <sgx_edger8r.h>
#include <stdint.h>
#include <atomic>
#include <queue>
#include <memory>
#include <string>

#include "Common/Constants.h"
#include "App/bookkeeping/database.h"
#include "App/types.h"
#include "App/request_parser.h"

class Monitor {
 private:
  OdbDriver *driver;
  const sgx_enclave_id_t eid;

  const std::atomic_bool &quit;

  /* process each request transaction */
  const string DATA_FIELD_NAME = "data";
  const string TX_HASH_FIELD_NAME = "transactionHash";

  static const int maxRetry = 5;
  static const int nothingToDoSleepSec = 5;
  bool isSleeping;

  bool send_tx;

  std::queue<std::unique_ptr<tc::RequestParser>> failed_requests;

 public:
  Monitor(OdbDriver *driver, sgx_enclave_id_t eid,
          const std::atomic_bool &quit)
      :
      send_tx(true),
      driver(driver),
      eid(eid),
      quit(quit),
      isSleeping(false) {}

  void dontSendResponse() { this->send_tx = false; }

  void loop();
  void _process_one_block(blocknum_t blocknum);
};

class NothingTodoException : public std::exception {
 public:
  char const *what() const noexcept override { return "Nothing to do"; }
};

#endif  // SRC_APP_MONITOR_H_
