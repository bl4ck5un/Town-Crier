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

#include "scrapers/ssalogin.h"
#include "error_codes.h"
#include "tls_client.h"
#include "scrapers.h"
#include "../pedersen_commit.h"

#include <vector>

using namespace std;

const std::string SSAScraper::HOST = "secure.ssa.gov";

err_code SSAScraper::handle(const uint8_t *req, size_t data_len, string *output) {
    return NO_ERROR;
}

err_code SSAScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
    return NO_ERROR;
}

/* Function that performs the HTTPS request and checks whether login cookie was returned */
int SSAScraper::perform_query(string token) {
  std::string url1 ("/myssa/mybec-api/baseEstimates");
  string h1[] = {"Connection: Close", "Accept: application/json, text/plain, */*", "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36", "Sec-Fetch-Site: same-origin", 
                  "Sec-Fetch-Mode: cors", "Referer: https://secure.ssa.gov/myssa/mybec-ui/", "Accept-Encoding: gzip, deflate",
                   "Accept-Language: en-US,en;q=0.9", "Cookie: PD-ID=" +token + ";"}; 
  vector<string> headers1(h1,h1 + 9); 
  HttpRequest httpRequest1(this->HOST, url1, headers1, true);
  HttpsClient httpClient1(httpRequest1);

  try {
    HttpResponse resp = httpClient1.getResponse();
    string body = resp.getContent();
    string birthday = parse_bday_response(body);
    // *status = NO_ERROR;
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient1.getError().c_str());
    httpClient1.close();
    // *status = WEB_ERROR;
  }


  std::string url2 ("/myssa/myhub-api/getAccesses");
  string h2[] = {"Connection: Close", "Accept: application/json, text/plain, */*", "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36", "Sec-Fetch-Site: same-origin", 
                "Sec-Fetch-Mode: cors", "Referer: https://secure.ssa.gov/myssa/myhub/", "Accept-Encoding: gzip, deflate",
                  "Accept-Language: en-US,en;q=0.9", "Cookie: PD-ID=" +token}; 
  vector<string> headers2(h2,h2 + 9); 

  HttpRequest httpRequest2(this->HOST, url2, headers2, true);
  HttpsClient httpClient2(httpRequest2);

  try {
    HttpResponse resp = httpClient2.getResponse();
    string body = resp.getContent();
    string legalName = parse_name_response(body);
    string username = parse_username_response(body);
    // *status = NO_ERROR;
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient2.getError().c_str());
    httpClient2.close();
    // *status = WEB_ERROR;
  }

  // perform some kind of packaging to send this off, not sure how this part works
  return 0;
}

std::string SSAScraper::parse_name_response(const string resp) {
  picojson::value user_info_obj;
  std::string err_msg = picojson::parse(user_info_obj, resp);
  if (!err_msg.empty() || !user_info_obj.is<picojson::object>()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return "ERROR";
  }
  std::string name;

  if (user_info_obj.contains("formattedName")
      && user_info_obj.get("name").is<string>()) {
    name = user_info_obj.get("name").get<string>();
    // not sure how to package these for output, awaiting instruction
  }else{
    LL_CRITICAL("Error parsing json object: %s", resp);
    return "ERROR";
  }

  LL_DEBUG("Name found: %s", name.c_str());
  return name;
}

std::string SSAScraper::parse_bday_response(const string resp) {
  picojson::value user_info_obj;
  std::string err_msg = picojson::parse(user_info_obj, resp);
  if (!err_msg.empty() || !user_info_obj.is<picojson::object>()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return "ERROR";
  }
  std::string birthday;

  if (user_info_obj.contains("criteria")
      && user_info_obj.get("criteria").is<picojson::object>()) {
    int month = (int) user_info_obj.get("criteria").get("dateOfBirthMonth").get<double>();
    int day = (int) user_info_obj.get("criteria").get("dateOfBirthDay").get<double>();
    int year= (int) user_info_obj.get("criteria").get("dateOfBirthYear").get<double>();
    birthday = std::to_string(month) + "/" + std::to_string(day) + "/" + std::to_string(year);
    // not sure how to package these for output, awaiting instruction
  }else{
    LL_CRITICAL("Error parsing json object: %s", resp);
    return "ERROR";
  }

  LL_DEBUG("Birthday: %s", birthday.c_str());
  return birthday;
}

std::string SSAScraper::parse_username_response(const string resp) {
  picojson::value user_info_obj;
  std::string err_msg = picojson::parse(user_info_obj, resp);
  if (!err_msg.empty() || !user_info_obj.is<picojson::object>()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return "ERROR";
  }
  std::string username;

  if (user_info_obj.contains("username")
      && user_info_obj.get("username").is<string>()) {
    username = user_info_obj.get("username").get<string>();
    // not sure how to package these for output, awaiting instruction
  }else{
    LL_CRITICAL("Error parsing json object: %s", resp);
    return "ERROR";
  }

  LL_DEBUG("Username: %s", username.c_str());
  return username;
}
