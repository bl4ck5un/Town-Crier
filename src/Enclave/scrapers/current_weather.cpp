//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,/weather?zip=11510&
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// TH/weather?zip=11510&E WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <Log.h>

#include "tls_client.h"
#include "current_weather.h"
#include "utils.h"

#define API_KEY "9b0ede9af16533e1557ad783c2dfe40d"

double WeatherScraper::parse_response(const char* resp) {
    
    double ret = 0;
    const char * end;
    const char * temp = resp;

    std::string buf_string(resp);
    std::size_t pos = buf_string.find("temperature\":");

    if (pos == std::string::npos){
        return 0.0;
    }

    temp += (pos + 13);
    end = temp;
    while (*end != ',') {
        end += 1;
    }
    ret = std::strtod(temp, NULL);
    return ret;
}

/* The Data is structured as follows:
 * 0x00 - 0x20 int 
 */
err_code WeatherScraper::handler(uint8_t *req, int data_len, int *resp_data){
    
    if (data_len != 32){
        LL_CRITICAL("data_len %d*32 is not 32",data_len / 32);
        return INVALID_PARAMS;
    }
    char lat[32] = {0};
    char lng[32] = {0};

    memcpy(lat, req, 0x20);
    memcpy(lng, req + 0x20, 0x20);
    
    double tmp;
    err_code ret = weather_current((const char*)lat, (const char*)lng, &tmp); 
    *resp_data = (int) tmp;
    return ret;
}

err_code WeatherScraper::weather_current(const char* lattitude, const char* longitude, double* r) {
    /* Null Checker */
    if (r == NULL){
        LL_CRITICAL("Error: Passed null pointers");
        return INVALID_PARAMS;
    }

    std::string query = "/forecast/9b0ede9af16533e1557ad783c2dfe40d/" + \
                        std::string(lattitude) + "," + std::string(longitude);
    LL_INFO("Query: %s", query.c_str());
    HttpRequest httpRequest("api.darksky.net", query, true);
    HttpsClient httpClient(httpRequest);

    try {
        HttpResponse response = httpClient.getResponse();
        *r = parse_response(response.getContent().c_str());
    }
    catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
        httpClient.close();
        return WEB_ERROR;
    }
    return NO_ERROR;
}

