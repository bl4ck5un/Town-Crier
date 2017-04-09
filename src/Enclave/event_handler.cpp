//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//


#include <Log.h>
#include <string>

#include "event_handler.h"
#include "scrapers.h"
#include "scrapers/Scraper.h"
#include "scrapers/flight.h"
#include "eth_transaction.h"


int handle_request(int nonce,
                   uint64_t id,
                   uint64_t type,
                   uint8_t *data,
                   int data_len,
                   uint8_t *raw_tx,
                   size_t *raw_tx_len) {
  bytes resp_data;
  uint64_t error_flag = 0;

  switch (type) {
    case TYPE_FINANCE_INFO: {
      // TODO
//        return stock_ticker_handler(nonce, id, type,
//            data, data_len,
//            raw_tx, raw_tx_len);
      break;
    }
    case TYPE_FLIGHT_INS: {
      FlightScraper flightHandler;
      int delay;
      switch (flightHandler.handler(data, data_len, &delay)) {
        case UNKNOWN_ERROR:
        case WEB_ERROR:
          return TC_INTERNAL_ERROR;
        // treat invalid_params as no_error
        case INVALID_PARAMS:
          error_flag = 1;
        case NO_ERROR:
          append_as_uint256(resp_data, delay, sizeof(delay));
          break;
      };
      break;
    }
    case TYPE_STEAM_EX:
      //TODO
//             int found = 0;
//             if (data_len != 6 * 32)
//             {
//                 LL_CRITICAL("data_len %d*32 is not 6*32", data_len / 32);
//                 return -1;
//             }
//             ret = handler_steam_exchange(data, data_len, &found);
//             if (ret == -1)
//             {
//                 LL_CRITICAL("%s returns %d", "handler_steam_exchange", ret);
//                 return -1;
//             }
// //            found = 1;
//             append_as_uint256(resp_data, found, sizeof ( found ));
//             resp_data_len = 32;
      break;
    case TYPE_CURRENT_VOTE: {
      double r1 = 0, r2 = 0, r3 = 0;
      long long time1, time2;

      rdtsc(&time1);
      yahoo_current("GOOG", &r1);
      rdtsc(&time2);
      LL_CRITICAL("Yahoo: %llu", time2 - time1);

      google_current("GOOG", &r3);
      rdtsc(&time1);
      LL_CRITICAL("Bloomberg: %llu", time1 - time2);

      google_current("GOOG", &r2);
      rdtsc(&time2);
      LL_CRITICAL("GOOGLE: %llu", time2 - time1);

      break;
    }
    default :LL_CRITICAL("Unknown request type: %" PRIu64, type);
      return -1;
      break;
  }

  // TODO: MAJOR: change type to larger type
  return form_transaction(nonce, 32, id, type, data, data_len, error_flag, resp_data, raw_tx, raw_tx_len);
}

//static int stock_ticker_handler(int nonce, uint64_t request_id, uint8_t request_type,
//                                const uint8_t *req, int req_len, uint8_t *raw_tx, size_t *raw_tx_len)
//{
//    int ret;
//    if (req_len != 64)
//    {
//        LL_CRITICAL("req_len is not 64");
//        return -1;
//    }
//
//    char* code = (char*)(req);
//    uint32_t date;
//    time_t epoch;
//    memcpy(&date, req + 64 - sizeof (uint32_t), sizeof (uint32_t));
//    date = swap_uint32(date);
//
//    epoch = date;
//    LL_NOTICE("Looking for %s at %lld", code, epoch);
//
//    int price = (int) get_closing_price(12, 3, 2014, "BABA");
//
//    LL_NOTICE("Closing pricing is %d", price);
//
//    bytes rr;
//    append_as_uint256(rr, price, sizeof (price));;
//
//    ret = form_transaction(nonce, 32,
//        request_id, request_type,
//        req, req_len,
//        rr,
//        raw_tx, raw_tx_len);
//
//    return ret;
//}