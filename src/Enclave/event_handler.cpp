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
#include "scrapers/utils.h"
#include "scrapers/stockticker.h"
#include "scrapers/UPS_Tracking.h"
#include "scrapers/steam2.h"
#include "scrapers/current_coinmarket.h"
#include "time.h"
#include "eth_transaction.h"
#include "eth_abi.h"
#include "Enclave_t.h"
#include "external/keccak.h"
#include "Constants.h"
#include "Log.h"

int handle_request(int nonce,
                   uint64_t id,
                   uint64_t type,
                   uint8_t *data,
                   int data_len,
                   uint8_t *raw_tx,
                   size_t *raw_tx_len)
{
    int ret;
    bytes resp_data;
    int resp_data_len = 0;
    int error_flag = 0;
/*
    printf_sgx("nonce: %d\n", nonce);
    printf_sgx("id: %llu\n", id);
    printf_sgx("type: %llu\n", type);
    printf_sgx("data len: %d\n", data_len);
    for (int i = 0; i < data_len; i++)
        printf_sgx("%u,", data[i]);
    printf_sgx("\n");
*/
    switch (type){
    case TYPE_FINANCE_INFO:
    {
        /* NEED TO FIGURE OUT HOW TO PARSE */ 
        int closingPrice;
        StockTickerScraper stockTickerScraper;
        switch (stockTickerScraper.handler(data, data_len, &closingPrice)){
            case INVALID_PARAMS:
                error_flag = 1;
                break;
            case WEB_ERROR:
                return TC_INTERNAL_ERROR;
            case NO_ERROR:
                LL_INFO("Closing pricing is %d", closingPrice);
                append_as_uint256(resp_data, closingPrice, sizeof(closingPrice));
                break;
        }
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
    {
        SteamScraper steamHandler;       
        int found;
        switch(steamHandler.handler(data, data_len, &found)){
            case UNKNOWN_ERROR:
            case WEB_ERROR:
                return TC_INTERNAL_ERROR;
            case INVALID_PARAMS:
                error_flag = 1;
                break;
            case NO_ERROR:
                append_as_uint256(resp_data, found, sizeof(found));
                break;
        }
        break;
    }
    
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
    case TYPE_UPS_TRACKING: {
      USPSScraper uSPSScraper;
      int pkg_status;
      switch(uSPSScraper.handler(data,data_len, &pkg_status)){
        case UNKNOWN_ERROR:
        case WEB_ERROR:
          return TC_INTERNAL_ERROR;
        // treat invalid_params as no_error
        case INVALID_PARAMS:
          error_flag = 1;
        case NO_ERROR:
          append_as_uint256(resp_data, pkg_status, sizeof(pkg_status));
          break;
      };
      break;
    }
    case TYPE_COINMARKET: {
      CoinMarket coinMarket;
      int coin_value;
      switch(coinMarket.handler(data, data_len, &coin_value)){
        case UNKNOWN_ERROR:
        case WEB_ERROR:
          return TC_INTERNAL_ERROR;
        case INVALID_PARAMS:
          error_flag = 1;
        case NO_ERROR:
          append_as_uint256(resp_data, coin_value, sizeof(coin_value));
          break;
      };
      break;
    } 
    default :LL_CRITICAL("Unknown request type: %" PRIu64, type);
      return -1;
      break;
  }

  // TODO: MAJOR: change type to larger type
  return form_transaction(nonce, 32, id, type, data, data_len, error_flag, resp_data, raw_tx, raw_tx_len);
}