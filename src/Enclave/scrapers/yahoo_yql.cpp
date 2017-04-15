//
// Created by fanz on 4/12/17.
//

#include "tls_client.h"
#include "yahoo_yql.h"
#include "Scraper.h"
#include "scraper_utils.h"
#include "Log.h"

#include <map>

err_code YahooYQL::execute(string &result) {
  string query = this->buildQuery();
  HttpRequest http_request("query.yahooapis.com", query, true);
  HttpsClient https_client(http_request);

  try {
    HttpResponse response = https_client.getResponse();
    LL_DEBUG("httpStatus=%d", response.getStatusCode());
    result = response.getContent();
    return NO_ERROR;
  } catch (std::runtime_error &e) {
    /* An HTTPS error has occured */
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", https_client.getError().c_str());
    https_client.close();
    return WEB_ERROR;
  }
}

string YahooYQL::buildQuery() {
  if (q.empty()) {
    throw invalid_argument("q is empty");
  }

  const string api_prefix = "/v1/public/yql?";
  map<string, string> parameters;

  parameters["q"] = url_encode(q.c_str());
  parameters["format"] = string(format == JSON ? "json" : "xml");
  parameters["diagnostics"] = "false";

  if (!env.empty()) {
    parameters["env"] = url_encode(env.c_str());
  }

  string query;

  for (map<string, string>::iterator it = parameters.begin(); it != parameters.end(); it++) {
    query += (query.empty() ? "" : "&") + (it->first + "=" + it->second);
  }

  return api_prefix + query;
}