//
// Created by fanz on 4/12/17.
//

#ifndef TOWN_CRIER_YAHOO_YQL_STOCK_H
#define TOWN_CRIER_YAHOO_YQL_STOCK_H

#include "Scraper.h"
#include "Log.h"
#include "yahoo_yql.h"
#include "external/picojson.h"
#include "commons.h"

#include "external/gmtime.h"

#include <string>
#include <time.h>

using namespace std;

#include <time.h>
#include <ctime>

void gmtime(const time_t *timer, struct tm*);

class YahooYQLStock : public Scraper {
 public:
/* The data is structured as follows:
 *      0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
 *      0x20 - 0x40 Month
 *      0x40 - 0x60 Day
 *      0x60 - 0x80 Year
 */
  err_code handler(uint8_t *req, size_t data_len, int *resp_data) {
    if (data_len != 32 * 2) {
      LL_CRITICAL("req_len %zu is not 2*32", data_len);
      return INVALID_PARAMS;
    }

    string symbol = string(req, req + 0x20);
    symbol = string(symbol.c_str());
    LL_INFO("symbol: %s", symbol.c_str());

    time_t unix_epoch = uint_bytes<time_t> (req + 0x20, 32);

    tc::ext::date date;
    gmtime_tc(&unix_epoch, date);

    LL_INFO("month: %d", date.month);
    LL_INFO("day: %d", date.day);
    LL_INFO("year: %d", date.year);

    return handle_one(symbol, date.year, date.month, date.day, resp_data);
  }

  err_code handle_one(string symbol, uint year, uint month, uint day, int* resp_data) {
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
        *resp_data = (int) closing_price;
        err = NO_ERROR;
      } else {
        err = WEB_ERROR;
      }
    }
    catch (exception& e) {
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

#endif //TOWN_CRIER_YAHOO_YQL_STOCK_H
