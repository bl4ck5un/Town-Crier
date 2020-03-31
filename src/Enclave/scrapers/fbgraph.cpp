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
#include "scrapers/fbgraph.h"
#include "error_codes.h"
#include "tls_client.h"
#include "scrapers.h"

using namespace std;

const std::string FBGraphScraper::HOST = "graph.facebook.com";

char *search(const char *buf, const char *search_string){
  return 0;
}

err_code FBGraphScraper::handle(const uint8_t *req, size_t data_len, string *output) {
    return NO_ERROR;
}
err_code FBGraphScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
    return NO_ERROR;
}

/* Function that performs the HTTPS request and return the xml file */
int FBGraphScraper::perform_query() {
  std::string url ("/v6.0/me?fields=id%2Cname%2Cfriends%2Cbirthday&access_token="+this->oauth);
  HttpRequest httpRequest(this->HOST, url, true);
  HttpsClient httpClient(httpRequest);

  try {
    HttpResponse resp = httpClient.getResponse();
    string body = resp.getContent();
    string parsed_response = parse_response(body);
    // *status = NO_ERROR;
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

void FBGraphScraper::set_oauth(std::string oauthStr){
    this->oauth = oauthStr;
}


std::string FBGraphScraper::parse_response(const string resp) {
  picojson::value user_info_obj;
  std::string err_msg = picojson::parse(user_info_obj, resp);
  if (!err_msg.empty() || !user_info_obj.is<picojson::object>()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return "ERROR";
  }
  std::string name;
  std::string birthday;
  int friendCount; 

  if (user_info_obj.contains("name")
      && user_info_obj.get("name").is<string>()
      && user_info_obj.contains("friends")
      && user_info_obj.get("friends").is<picojson::object>()
      && user_info_obj.contains("birthday")
      && user_info_obj.get("birthday").is<string>()) {
    name = user_info_obj.get("name").get<string>();
    birthday = user_info_obj.get("birthday").get<string>();
    friendCount = (int) user_info_obj.get("friends").get("summary").get("total_count").get<double>();
    // not sure how to package these for output, awaiting instruction
  }else{
    LL_CRITICAL("Error parsing json object: %s", resp);
    return "ERROR";
  }

  LL_DEBUG("Friends number: %d", friendCount);
  LL_DEBUG("User number: %s", name.c_str());
  LL_DEBUG("Birthday: %s", birthday.c_str());
  return "henlo";
}