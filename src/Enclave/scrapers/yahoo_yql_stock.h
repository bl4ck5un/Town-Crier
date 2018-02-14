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

//
// Created by fanz on 4/12/17.
//

#ifndef SRC_ENCLAVE_SCRAPERS_YAHOO_YQL_STOCK_H_
#define SRC_ENCLAVE_SCRAPERS_YAHOO_YQL_STOCK_H_

#include <string>
#include <time.h>
#include <ctime>

#include "Scraper.h"
#include "../log.h"
#include "yahoo_yql.h"
#include "external/picojson.h"
#include "commons.h"

#include "external/gmtime.h"

using namespace std;

void gmtime(const time_t *timer, struct tm *);

class YahooYQLStock : public Scraper {
 public:
/* The data is structured as follows:
 *      0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
 *      0x20 - 0x40 Month
 *      0x40 - 0x60 Day
 *      0x60 - 0x80 Year
 */
  err_code handle(const uint8_t *req, size_t data_len, int *resp_data) {
    if (data_len != 32 * 2) {
      LL_CRITICAL("req_len %zu is not 2*32", data_len);
      return INVALID_PARAMS;
    }

    string symbol = string(req, req + 0x20);
    symbol = string(symbol.c_str());
    LL_INFO("symbol: %s", symbol.c_str());

    time_t unix_epoch = uint_bytes<time_t>(req + 0x20, 32);

    // if longer than 2100-1-1
    if (unix_epoch > 0xF4865700) {
      LL_CRITICAL("unix_epoch %ld is too far to the future", unix_epoch);
      return INVALID_PARAMS;
    }

    tc::ext::date date;
    gmtime_tc(&unix_epoch, date);

    LL_INFO("month: %d", date.month);
    LL_INFO("day: %d", date.day);
    LL_INFO("year: %d", date.year);

    return handle_one(symbol, date.year, date.month, date.day, resp_data);
  }

  err_code handle_one(string symbol, int year, int month, int day, int *resp_data) {
    LL_DEBUG("handling %s %d-%d-%d", symbol.c_str(), year, month, day);

    if (symbol.empty() || day > 31 || year > 2017 || month > 12) {
      return INVALID_PARAMS;
    }

    struct tm queried_date;
    queried_date.tm_year = year - 1900;
    queried_date.tm_mon = month - 1;
    queried_date.tm_mday = day;

    char _date_buf[20];
    strftime(_date_buf, sizeof _date_buf, "%F", &queried_date);

    LL_DEBUG("date string is %s", _date_buf);

    char q[1024];
    snprintf(q,
             sizeof q,
             "select * from yahoo.finance.historicaldata where symbol = \"%s\" and startDate = \"%s\" and endDate = \"%s\"",
             symbol.c_str(),
             _date_buf,
             _date_buf);

    LL_DEBUG("query created %s", q);

    YahooYQL yql(q, YahooYQL::JSON, "store://datatables.org/alltableswithkeys");

    string result;
    err_code err = yql.execute(result);
    if (err != NO_ERROR) {
      return err;
    }

    LL_INFO("result is %s", result.c_str());

    picojson::value _result_json;
    string err_msg = picojson::parse(_result_json, result);
    if (!err_msg.empty() || !_result_json.is<picojson::object>()) {
      LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
      return WEB_ERROR;
    }

    // obtain a const reference to the map, and print the contents
    try {
      picojson::value _result = _result_json.get("query").get("results");
      if (!_result.evaluate_as_boolean()) {
        LL_CRITICAL("%s contains no result", _result.to_str().c_str());
        return INVALID_PARAMS;
      }

      double closing_price = 0;
      picojson::value _closing_json = _result.get("quote").get("Close");
      if (_closing_json.is<string>()) {
        closing_price = strtod(_closing_json.get<string>().c_str(), NULL);
        LL_INFO("closing price is %f", closing_price);
        *resp_data = static_cast<int>(closing_price);
        err = NO_ERROR;
      } else {
        err = WEB_ERROR;
      }
    }
    catch (exception &e) {
      LL_CRITICAL("Error happened: %s", e.what());
      err = WEB_ERROR;
    }
    catch (...) {
      LL_CRITICAL("caught unknown exception");
      err = UNKNOWN_ERROR;
    }
    return err;
  }
};

#endif  // SRC_ENCLAVE_SCRAPERS_YAHOO_YQL_STOCK_H_
