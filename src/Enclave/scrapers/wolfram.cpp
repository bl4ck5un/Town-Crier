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

/* Define wolfrasm scraper constant */
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include "../log.h"

#include "wolfram.h"
#include "utils.h"
#include "tls_client.h"

using namespace std;

const std::string WolframScraper::HOST = "api.wolframalpha.com";
const std::string WolframScraper::APPID = "A8V8R2-523WY42ULW";

/* WolframResults constructor */
WolframQueryResult::WolframQueryResult(string xml) {
  this->xml = xml;
}

/* Return a pointer to the raw xml file */
string WolframQueryResult::get_raw_data() {
  return this->xml;
}

void WolframScraper::create_query(std::string query) {
  for (std::string::iterator it = query.begin(); it != query.end(); ++it) {
    if (*it == ' ') {
      *it = '+';
    }
  }

  this->url = "/v1/result?appid=" + this->APPID + "&i=" + query;
}

err_code WolframScraper::handle(const uint8_t *req, size_t data_len, string *output) {
  if (data_len != 32) {
    LL_CRITICAL("incorrect length: %zu (wants 32)", data_len);
    return INVALID_PARAMS;
  }

  string query(string(req, req + 0x20).c_str());
  LL_DEBUG("got query %s (len=%zu)", query.c_str(), query.length());

  std::replace(query.begin(), query.end(), ' ', '+');
  this->url = "/v1/result?appid=" + this->APPID + "&i=" + query;

  LL_DEBUG("wolfram url => %s", this->url.c_str());

  WolframQueryResult results = perform_query();

  output->clear();
  *output = results.get_raw_data();
  return NO_ERROR;
}

err_code WolframScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
  return NO_ERROR;
}

/* Function that performs the HTTPS request and return the xml file */
WolframQueryResult WolframScraper::perform_query() {
  HttpRequest httpRequest(this->HOST, this->url, true);
  HttpsClient httpClient(httpRequest);

  try {
    HttpResponse resp = httpClient.getResponse();
    // *status = NO_ERROR;
    WolframQueryResult wolframQueryResult(resp.getContent());
    return wolframQueryResult;
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    // *status = WEB_ERROR;
  }
  /* Hacky: add exception handling */
  WolframQueryResult tmp("");
  return tmp;
}

void WolframScraper::set_qtype(int type) {
  switch (type) {
    case 1:
      this->wolframQueryType = SIMPLE;
      break;
    default:
      this->wolframQueryType = GENERAL;
      break;
  }
}
