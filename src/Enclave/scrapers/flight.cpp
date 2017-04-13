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
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "flight.h"
#include "Debug.h"
#include "Log.h"
#include "utils.h"
#include "../../Common/Constants.h"
#include "../external/slre.h"
#include "tls_client.h"


/* Define flight scraper specific constants */

///*
//    A few notes on integration
//    - username & password should be passed as an Authorization header field, with Base64(user:password)
//      as its content.
//    - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
//*/
#define STR1(x)  #x
#define STR(x)  STR1(x)

#define AUTH_CODE "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3"
#define HOST "Host: flightxml.flightaware.com"
#define SECOND_PER_MIN 60
#define MAX_DELAY_MIN 30
#define NUM_ENTRY 30
#define HOW_MANY "&howMany=" STR(NUM_ENTRY)

/*Class used to handle the flight insurance */
std::string FlightScraper::uint64_to_string(uint64_t value) {
  // length of 2**64 - 1, +1 for nul.
  char buf[21];
  snprintf(buf, sizeof buf, "%"PRIu64, value);
  std::string str(buf);
  return str;
}

flight_error FlightScraper::parse_response(const char *resp, int *delay, uint64_t unix_epoch_time) {

  //Find the scheduled departure time
  std::string buff(resp);
  std::string delimeter = "\"filed_departuretime\":" + uint64_to_string(unix_epoch_time);
  std::size_t pos = buff.find(delimeter);

  //Corner Case: Flight was not found
  if (pos > buff.length()) {
    LL_DEBUG("Invalid");
    return INVALID;
  }

  //Find the actual departure time
  std::string delimeter2 = "actualdeparturetime\":";
  std::size_t pos2 = buff.find(delimeter2, pos);
  std::string token = buff.substr(pos2 + delimeter2.length(), pos2 + delimeter2.length() + 10);

  uint64_t actual_depart_time = atoi(token.c_str());
  LL_DEBUG("actualdeparturetime: %llu", actual_depart_time);
  LL_DEBUG("filed_departuretime: %llu", unix_epoch_time);
  LL_DEBUG("diff: %llu", actual_depart_time - unix_epoch_time);

  //Case: Flight has not yet departed
  if (actual_depart_time == 0) {
    LL_DEBUG("NOT_DEPARTED");
    return NOT_DEPARTED;
  }
  //Case: Flight was cancelled //RETURN MAX_INT DELAY
  if (actual_depart_time == -1) {
    *delay = 2147483643; //Delay is some very large number
    LL_DEBUG("CANCELLED");
    return CANCELLED;
  }
  //Case: Flight Departed but delayed
  if (actual_depart_time - unix_epoch_time >= MAX_DELAY_MIN * SECOND_PER_MIN) {
    LL_DEBUG("DELAYED");
    *delay = actual_depart_time - unix_epoch_time;
    return DELAYED;
  }
    //Case: Flight was not delayed
  else {
    *delay = 0;
    LL_DEBUG("DEPARTED");
    return DEPARTED;
  }
}

//
///*
//    date:   YYYYMMDD
//    time:   HHmm
//    flight: ICAO flight numbers
//    resp:   set to the number of minutes late/early
//
//    return: 0 if OK, -1 if no data found or flight still enroute
//
//    date and time in Zulu/UTC
//*/
flight_error FlightScraper::get_flight_delay(uint64_t unix_epoch_time, const char *flight, int *resp) {
  /* Invalid user input */
  if (flight == NULL || resp == NULL) {
    return INVALID;
  }

  //Build header for https request
  std::vector<string> header;
  header.push_back(AUTH_CODE);
  header.push_back(HOST);

  //Construct the query
  std::string query =
      "/json/FlightXML2/FlightInfoEx?ident=" + std::string(flight) +
          HOW_MANY +
          "&offset=0 HTTP/1.1";

  HttpRequest httpRequest("flightxml.flightaware.com", query, header);
  HttpsClient httpClient(httpRequest);
  flight_error ret;
  try {
    HttpResponse response = httpClient.getResponse();
    ret = parse_response(response.getContent().c_str(), resp, unix_epoch_time);
  } catch (std::runtime_error &e) {
    /* An HTTPS error has occured */
    LL_CRITICAL("Https error: %s", e.what());
    LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    httpClient.close();
    return HTTP_ERROR;
  }
  return ret;
}

/*  The Data is structured as follows:
 *      0x00 - 0x20 string flight_number
 *      0x20 - 0x40 uint64 unix_epoch
 */
err_code FlightScraper::handler(uint8_t *req, size_t data_len, int *resp_data) {

  if (data_len != 2 * 32) {
    LL_CRITICAL("Data_len %zu*32 is not 2*32", data_len / 32);
    return INVALID_PARAMS;
  }
  int delay;
  char flight_number[33] = {0};
  memcpy(flight_number, req, 0x20);

  char *flighttime = (char *) req + 0x20;
  uint64_t unix_epoch = (uint64_t) strtol(flighttime, NULL, 10);

  LL_DEBUG("unix_epoch=%lld, flight_number=%s", unix_epoch, flight_number);

  switch (get_flight_delay(unix_epoch, flight_number, &delay)) {
    case INVALID:
      *resp_data = -1;
      return INVALID_PARAMS;

    case NOT_FOUND:
      *resp_data = -1;
      return INVALID_PARAMS;

    case HTTP_ERROR:
      *resp_data = -1;
      return WEB_ERROR;

    case DEPARTED:
    case DELAYED:
    case CANCELLED:
      //TODO: why?
    case NOT_DEPARTED:
      *resp_data = delay;
      return NO_ERROR;
    default:
      return UNKNOWN_ERROR;
  }
}
