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
#include <string.h>

#include <string>

#include "../log.h"
#include "tls_client.h"
#include "current_weather.h"
#include "utils.h"
#include "yahoo_yql.h"
#include "../external/picojson.h"

#define API_KEY "9b0ede9af16533e1557ad783c2dfe40d"

using namespace std;

err_code WeatherScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
  if (data_len != 1 * 32) {
    LL_CRITICAL("data_len %zu*32 is not 32", data_len / 32);
    return INVALID_PARAMS;
  }

  string query(string(req, req + 0x20).c_str());
  this->WeatherQueryType = CITYNAME;
  LL_INFO("query: %s", query.c_str());
  err_code ret = weather_current(query, resp_data);
  return ret;
}

err_code WeatherScraper::weather_current(const string &request, int *r) {
  /* Null Checker */
  if (request.size() == 0 || r == NULL) {
    LL_CRITICAL("Error: Passed null pointers");
    return INVALID_PARAMS;
  }
  string query = this->construct_query(request);
  LL_INFO("query sent: %s", query.c_str());

  YahooYQL yahooYQL(query, YahooYQL::JSON, "store://datatables.org/alltableswithkeys");
  string resp;
  err_code ret = yahooYQL.execute(resp);
  if (ret == WEB_ERROR) {
    LL_CRITICAL("failed to get response");
    return WEB_ERROR;
  }

  LL_INFO("response: %s", resp.c_str());

  picojson::value v;
  std::string err = picojson::parse(v, resp);
  if (!err.empty()) {
    LL_CRITICAL("Error in picojson");
    return INVALID_PARAMS;
  }

  if (!v.is<picojson::object>()) {
    LL_CRITICAL("JSON is not an object");
    return INVALID_PARAMS;
  }

  const picojson::value::object &obj = v.get<picojson::object>();
  picojson::value v1 = obj.find("query")->second;
  const picojson::object &obj2 = v1.get<picojson::object>();
  picojson::value v2 = obj2.find("results")->second;
  if (v2.is<picojson::null>()) {
    LL_CRITICAL("result is null");
    return WEB_ERROR;
  }
  const picojson::object &obj3 = v2.get<picojson::object>();
  picojson::value v3 = obj3.find("channel")->second;
  const picojson::object &obj4 = v3.get<picojson::object>();
  picojson::value v4 = obj4.find("item")->second;
  const picojson::object &obj5 = v4.get<picojson::object>();
  picojson::value v5 = obj5.find("condition")->second;
  const picojson::object &obj6 = v5.get<picojson::object>();
  picojson::value v6 = obj6.find("temp")->second;
  const std::string &temperature = v6.get<std::string>();

  LL_INFO("temp: %s", temperature.c_str());
  *r = atoi(temperature.c_str());
  return ret;
}

/* For testing purposes only */
void WeatherScraper::set_qtype(int type) {
  switch (type) {
    case 1:
      this->WeatherQueryType = WOEID;
      break;
    case 2:
      this->WeatherQueryType = CITYNAME;
      break;
    case 3:
      this->WeatherQueryType = LATLONG;
      break;
    default:
      LL_CRITICAL("Error");
      break;
  }
}

string WeatherScraper::construct_query(string request) {
  string q;
  switch (this->WeatherQueryType) {
    case WOEID:
      q = "select item.condition from weather.forecast where woeid = " + request;
      break;
    case CITYNAME:
      q = "select item.condition from weather.forecast where woeid in (select woeid from geo.places(1) where text=\""
          + request + "\")";
      break;
    case LATLONG:
      q = "";
      break;
  }
  return q;
}
