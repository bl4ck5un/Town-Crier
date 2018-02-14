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

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include "tls_client.h"
#include "scrapers/stock_ticker.h"
#include "../log.h"

#include "scrapers/yahoo_yql_stock.h"
#include "scrapers/yahoo_yql.h"

static int test(const uint8_t *data) {
  YahooYQLStock yahooYQLStock;
  int closing_price;
  err_code err = yahooYQLStock.handle(data, 32 * 2, &closing_price);
  switch (err) {
    case NO_ERROR:
      LL_INFO("closingPrice is %d", closing_price);
      return 0;
    case WEB_ERROR:
      LL_INFO("Returned unexpected WEB_ERROR");
      return -1;
    case INVALID_PARAMS:
      LL_INFO("Returned unexpected INVALID_PARAMS");
      return 0;
    default:
      return -1;
  }
}

int stockticker_self_test() {
  try {
    string symbol = "YHOO";
    symbol.append(32 - symbol.length(), 0);
    string unix_epoch = "\x58\x3E\x8C\x80";
    unix_epoch.insert(unix_epoch.begin(), 32 - unix_epoch.length(), '\0');

    string data = symbol + unix_epoch;
    dump_buf("data", (const unsigned char *) data.c_str(), data.size());

    test((const uint8_t *) data.c_str());

    bytes32 ticker("AAPL");
    bytes32 unix_time(1492041600);
    bytes data_(ticker, unix_time);

    dump_buf("data", &data_[0], data_.size());

    test(&data_[0]);
    return 0;
  }
  catch (const std::exception &e) {
    LL_CRITICAL("exception: %s", e.what());
    return -1;
  }
  catch (...) {
    LL_CRITICAL("unknown exp");
    return -1;
  }

}