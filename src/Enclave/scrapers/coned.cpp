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

#include "scrapers/coned.h"
#include "error_codes.h"
#include "tls_client.h"
#include "scrapers.h"
#include <vector>

using namespace std;

const std::string ConEdScraper::HOST = "www.coned.com";

err_code ConEdScraper::handle(const uint8_t *req, size_t data_len, string *output) {
    return NO_ERROR;
}

err_code ConEdScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
    return NO_ERROR;
}

/* Function that performs the HTTPS request and checks whether login cookie was returned */
int ConEdScraper::perform_query(string cookie) { // preprocess cookie to desired form
  std::string url ("/en/accounts-billing/dashboard?account=005100006720028&tab1=overview-1");
  string h[] = {"Connection: close", "Upgrade-Insecure-Requests: 1", "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36", 
                "Sec-Fetch-User: ?1", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3"
                "Sec-Fetch-Site: cross-site", "Sec-Fetch-Mode: navigate", "Accept-Encoding: gzip, deflate", "Accept-Language: en-US,en;q=0.9", "Cookie: " + cookie}; 
  vector<string> headers(h,h + 10); 
  HttpRequest httpRequest(this->HOST, url, headers, true);
  HttpsClient httpClient(httpRequest);

  try {
    HttpResponse resp = httpClient.getResponse();
    string body = resp.getContent();
    string birthday = parse_name_response(body);
    // *status = NO_ERROR;
  }
  catch (std::runtime_error &e) {
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    // *status = WEB_ERROR;
  }
  // perform some kind of packaging to send this off, not sure how this part works
  return 0;
}

std::string ConEdScraper::parse_name_response(const string resp) {
    std::string output("placeholder");
    std::string dropdown("js-name-selector");
    std::size_t found = resp.find(dropdown);
    if(found != std::string::npos){
        while (resp[found] != '>'){
            found++;
        }
        int first_index = found+1;
        int length = 0;
        found++;
        while (resp[found] != '<'){
            length++;
            found++;
        }
        std::string result = resp.substr(first_index, length);
        return result;
    }else{
        LL_CRITICAL("ERROR PARSING RESPONSE");
        return "failure, parsing did not work.";
    }
    return output;
}
