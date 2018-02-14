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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "external/csv_parser.hpp"
#include "../log.h"
#include "stock_ticker.h"
#include "tls_client.h"

using namespace std;

/* Implement the stockQuery class **/
StockQuery::StockQuery(int month, int day, int year, std::string symbol) {
  this->day = day;
  this->month = month - 1;
  this->year = year;
  this->symbol = symbol;
}

void StockQuery::SetDay(int day) {
  this->day = day;
}

void StockQuery::SetMonth(int month) {
  this->month = month;
}

void StockQuery::SetYear(int year) {
  this->year = year;
}

void StockQuery::SetSymbol(std::string symbol) {
  this->symbol = symbol;
}

std::string StockQuery::GetUrl() {
  char tmp[100];
  snprintf(tmp, sizeof(tmp), \
       "ichart.yahoo.com/table.csv?s=%s&a=%d&b=%d&c=%d&d=%d&e=%d&f=%d&g=d&ignore=.csv", \
       this->symbol.c_str(), this->month, this->day, this->year, \
       this->month, this->day, this->year);  // Datasource required the month to be subtracted by 1
  std::string str(tmp);
  return str;
}

/** Implement the StockTickerScraper class **/
/* Note Still not sure why we decrease the month by 1 */
void StockTickerScraper::CreateQuery(int month, int day, int year, std::string symbol) {
  this->query.SetDay(day);
  this->query.SetMonth(month);
  this->query.SetYear(year);
  this->query.SetSymbol(symbol);
}

/* The data is structured as follows:
 *      0x00 - 0x20 Symbol (i.e GOOG, APPL, etc)
 *      0x20 - 0x40 Month
 *      0x40 - 0x60 Day
 *      0x60 - 0x80 Year
 */
err_code StockTickerScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
  if (data_len != 32 * 4) {
    LL_CRITICAL("req_len %zu is not 4*32", data_len / 32);
    return INVALID_PARAMS;
  }

  char symbol[35] = {0};
  memcpy(symbol, req, 4);
  string str(symbol);
  LL_INFO("symbol: %s", symbol);

  int month = strtol(reinterpret_cast<const char *>(req + 0x20), NULL, 10);
  int day = strtol(reinterpret_cast<const char *>(req + 0x40), NULL, 10);
  int year = strtol(reinterpret_cast<const char *>(req + 0x60), NULL, 10);

  LL_INFO("month: %d", month);
  LL_INFO("day: %d", day);
  LL_INFO("year: %d", year);

  CreateQuery(day, month, year, symbol);
  StockTickerParser parser = QueryWebsite();
  if (parser.GetErrorCode() == WEB_ERROR) {
    return WEB_ERROR;
  }

  string resp(parser.GetResponse());

  std::vector<std::string> _rows;

  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = resp.find('\n', prev)) != std::string::npos) {
    _rows.push_back(resp.substr(prev, pos - prev));
    prev = pos + 1;
  }

  // To get the last substring (or only, if delimiter is not found)
  _rows.push_back(resp.substr(prev));

  CSV_Parser csv_parser;
  KEY_VAL_FIELDS _price_chart;
  CSV_FIELDS _price_chart_hdr;
  csv_parser.parse_line(_rows[0], _price_chart_hdr);
  csv_parser.parse_line(_rows[1], _price_chart_hdr, _price_chart);

  for (KEY_VAL_FIELDS::iterator it = _price_chart.begin(); it != _price_chart.end(); it++) {
    LL_DEBUG("%s -> %s", it->first.c_str(), it->second.c_str());
  }

  double closing_price = atof(_price_chart["Close"].c_str());
  *resp_data = static_cast<int>(closing_price);
  return NO_ERROR;

//  double closingPrice = parser.GetClosingPrice();
//    *resp_data = (int) closingPrice;
//    return NO_ERROR;
}

/* Query the ichar.yahoo website, returns a StockTickerParser which can be 
    used to parse the response
*/
StockTickerParser StockTickerScraper::QueryWebsite() {
  HttpRequest httpRequest("ichart.yahoo.com", this->query.GetUrl().c_str(), false);
  HttpsClient httpClient(httpRequest);

  try {
    HttpResponse response = httpClient.getResponse();
    LL_DEBUG("httpStatus=%d", response.getStatusCode());
    return StockTickerParser(response.getContent().c_str(), NO_ERROR);
  } catch (std::runtime_error &e) {
    /* An HTTPS error has occured */
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    return StockTickerParser(NULL, WEB_ERROR);
  }
}


/** Implement the StockTickerParser class **/

/* Implement the constructor */
StockTickerParser::StockTickerParser(const char *resp, err_code err) {
  this->rawResponse = resp;
  this->error = err;
}

/* Returns the raw csv file */
const char *StockTickerParser::GetResponse() {
  return this->rawResponse;
}

/* Returns the error code */
err_code StockTickerParser::GetErrorCode() {
  return this->error;
}

/* returns the closing price of the stock */
double StockTickerParser::GetClosingPrice() {
  std::vector<std::string> _rows;
  std::string resp = GetResponse();

  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = resp.find('\n', prev)) != std::string::npos) {
    _rows.push_back(resp.substr(prev, pos - prev));
    prev = pos + 1;
  }

  // To get the last substring (or only, if delimiter is not found)
  _rows.push_back(resp.substr(prev));

  CSV_Parser csv_parser;
  KEY_VAL_FIELDS _price_chart;
  CSV_FIELDS _price_chart_hdr;
  csv_parser.parse_line(_rows[0], _price_chart_hdr);
  csv_parser.parse_line(_rows[1], _price_chart_hdr, _price_chart);

  for (KEY_VAL_FIELDS::iterator it = _price_chart.begin(); it != _price_chart.end(); it++) {
    LL_DEBUG("%s -> %s", it->first.c_str(), it->second.c_str());
  }

  double closing_price = atof(_price_chart["Close"].c_str());
  return closing_price;
}
