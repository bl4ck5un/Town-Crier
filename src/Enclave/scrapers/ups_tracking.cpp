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
#include <string>
#include <vector>

#include "ups_tracking.h"
#include "utils.h"
#include "tls_client.h"
#include "../../Common/Constants.h"
#include "../external/picojson.h"
#include "../log.h"

using namespace std;
/* Define USPS Scraper specific constants */
#define USPS_API "stg-production.shippingapis.com"
#define USERID "063CORNE4274"

const std::string USPSScraper::APIKEY = "NkRXcVAzeERlRUZPQmtlckJpczRCZzo=";
const std::string USPSScraper::HOST = "api.easypost.com";

/* Method used to handle any UPSP data request
 * The Data is strcutured as follows
 *    0x00 - 0x20 string tracking_number
 */
err_code USPSScraper::handle(const uint8_t *req, size_t len, int *resp_data) {
  if (len != 2 * 32) {
    LL_CRITICAL("Data_len %zu*32 is not 2*32", len / 32);
    return INVALID_PARAMS;
  }
  // Parse the raw array to get the required params
  // char tracking_num[32] = {0};
  // char carrier_name[32] = {0};
  std::string tracking_num(req, req + 0x20);
  std::string carrier_name(req + 0x20, req + 0x40);
  // memcpy(tracking_num, req, 0x20);
  // memcpy(carrier_name, req + 0x20, 0x20);

  LL_DEBUG("Tracking Number is: %s", tracking_num.c_str());

  return ups_tracking(tracking_num, carrier_name, resp_data);
}

err_code USPSScraper::ups_tracking(const std::string &tracking_num, const std::string &carrier_name, int *status) {
  if (tracking_num.size() == 0) {
    LL_CRITICAL("Error: Passed in NULL Pointer");
    *status = -1;
    return INVALID_PARAMS;
  }

  /* Build the query */
  std::string query = "/v2/trackers?tracker[tracking_code]=" + tracking_num + "&tracker[carrier]=" + carrier_name;
  std::vector<string> header;
  LL_INFO("query: %s", query.c_str());

  std::string auth = "Authorization: Basic " + this->APIKEY;
  LL_INFO("auth: %s", auth.c_str());
  header.push_back(auth);

  HttpRequest httpRequest(this->HOST, query, header, true);
  HttpsClient httpClient(httpRequest);
  std::string result;
  try {
    HttpResponse response = httpClient.getResponse();
    result = parse_response(response.getContent());
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    *status = -1;
    return WEB_ERROR;
  }

  // return an int according to the result. E.g. 1 for delivered, etc.
  if (result.compare("Package not found") == 0) {
    *status = PACKAGE_NOT_FOUND;
    return NO_ERROR;
  }
  if (result.compare("pre_transit") == 0) {
    *status = PRE_TRANSIT;
    return NO_ERROR;
  }
  if (result.compare("Delivered") == 0) {
    *status = DELIVERED;
    return NO_ERROR;
  }
  if (result.compare("Order processed") == 0) {
    *status = ORDER_PROCESSED;
    return NO_ERROR;
  }
  if (result.compare("Shipped") == 0) {
    *status = SHIPPED;
    return NO_ERROR;
  }
  if (result.compare("In transit") == 0) {
    *status = IN_TRANSIT;
    return NO_ERROR;
  }
  if (result.compare("Out for delivery") == 0) {
    *status = OUT_FOR_DELIVERY;
    return NO_ERROR;
  } else {
    LL_CRITICAL("failed to get information\n");
    return INVALID_PARAMS;
  }
}

std::string USPSScraper::parse_response(const string resp) {
  LL_INFO("resp: %s", resp.c_str());

  picojson::value v;
  std::string err = picojson::parse(v, resp);
  if (!err.empty()) {
    LL_CRITICAL("Error in picojson");
    throw CannotParseResponse(resp);
  }
  if (!v.is<picojson::object>()) {
    LL_CRITICAL("JSON is not an object");
    throw CannotParseResponse(resp);
  }

  try {
    const picojson::value::object &obj = v.get<picojson::object>();

    picojson::value v1 = obj.find("trackers")->second;
    const picojson::value::array &tr_list = v1.get<picojson::array>();
    const picojson::value::object &obj2 = tr_list.at(0).get<picojson::object>();

    picojson::value v2 = obj2.find("tracking_details")->second;
    const picojson::value::array &tracking_history = v2.get<picojson::array>();

    const picojson::value::object &obj3 = tracking_history.at(tracking_history.size() - 1).get<picojson::object>();
    picojson::value v3 = obj3.find("status")->second;
    const std::string &pkg_status = v3.get<std::string>();
    LL_INFO("status of package: %s", pkg_status.c_str());

    return pkg_status;
  }
  catch (const exception& e) {
    LL_CRITICAL("exception: %s", e.what());
    throw CannotParseResponse(resp);
  }
}

/* Code used for testing
int main(){
  char trackID[] = "1Z1aasdfa581202007873";
  char* tID = trackID;
  std::string status; 
  ups_current(trackID, &status);
  cout << status << "\n";
  return 0; 

}
*/ 
