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
#include "../external/picojson.h"

#include "scrapers/fblogin.h"
#include "error_codes.h"
#include "tls_client.h"
#include "scrapers.h"

using namespace std;

const std::string FBScraper::HOST = "graph.facebook.com";

char *search(const char *buf, const char *search_string){
  return 0;
}

err_code FBScraper::handle(const uint8_t *req, size_t data_len, string *output) {
    return NO_ERROR;
}

err_code FBScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
    return NO_ERROR;
}

/* Function that performs the HTTPS request and return the xml file */
int FBScraper::perform_query() {
  std::string url ("/v6.0/me?access_token="+this->oauth);
  HttpRequest httpRequest(this->HOST, url, true);
  HttpsClient httpClient(httpRequest);
  std::string response;
  try {
    HttpResponse resp = httpClient.getResponse();
    // *status = NO_ERROR;
    response = parse_response(resp.getContent());
    return 0;
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    // *status = WEB_ERROR;
  }
  return 0;
}

void FBScraper::set_oauth(std::string oauthStr){
    this->oauth = oauthStr;
}

std::string FBScraper::parse_response(const string resp) {
  return "henlo";
}
