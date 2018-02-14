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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <vector>

#include "flight.h"
#include "debug.h"
#include "../log.h"
#include "utils.h"
#include "Constants.h"
#include "external/slre.h"
#include "tls_client.h"

using std::string;

#include "commons.h"
#include "hybrid_cipher.h"

const char *FlightScraper::HOST = "Host: flightxml.flightaware.com";
const char
    *FlightScraper::AUTH_CODE = "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3";

#include "external/picojson.h"
using std::vector;

flight_error FlightScraper::parse_response(const string &resp, int *delay, uint64_t unix_epoch_time) {
  LL_DEBUG("parsing from %s", resp.c_str());

  // http://flightxml.flightaware.com/soap/FlightXML2/doc#type_FlightInfoExStruct
  picojson::value flight_info_struct;
  string err = picojson::parse(flight_info_struct, resp);
  if (!err.empty() || !flight_info_struct.is<picojson::object>()) {
    LL_CRITICAL("can't parse %s", resp.c_str());
    return HTTP_ERROR;
  }

  if (flight_info_struct.contains("error")) {
    if (flight_info_struct.get("error").is<string>()) {
      err = flight_info_struct.get("error").get<string>();
    }

    LL_CRITICAL("%s", err.c_str());
    return NOT_FOUND;
  }

  // http://flightxml.flightaware.com/soap/FlightXML2/doc#type_FlightExStruct
  uint64_t actual_depart_time;
  picojson::value flight_ex_struct = flight_info_struct.get("FlightInfoExResult").get("flights");
  if (flight_ex_struct.is<picojson::array>()) {
    picojson::value _flight = flight_ex_struct.get<picojson::array>()[0];
    if (_flight.get("actualdeparturetime").is<double>()) {
      actual_depart_time = (uint64_t) _flight.get("actualdeparturetime").get<double>();
    } else {
      // actualdepartime is not double
      return HTTP_ERROR;
    }
  } else {
    LL_CRITICAL("no flight info found");
    return NOT_FOUND;
  }

  LL_DEBUG("actualdeparturetime: %"
               PRIu64, actual_depart_time);
  LL_DEBUG("filed_departuretime: %"
               PRIu64, unix_epoch_time);
  LL_DEBUG("diff: %"
               PRIu64, actual_depart_time - unix_epoch_time);

  // Case: Flight has not yet departed
  if (actual_depart_time == 0) {
    LL_DEBUG("NOT_DEPARTED");
    return NOT_DEPARTED;
  }
  // Case: Flight was cancelled //RETURN MAX_INT DELAY
  if (actual_depart_time == -1) {
    *delay = 2147483643;  // Delay is some very large number
    LL_DEBUG("CANCELLED");
    return CANCELLED;
  }
  // Case: Flight Departed but delayed
  if (actual_depart_time - unix_epoch_time >= MAX_DELAY_MIN * 60) {
    LL_DEBUG("DELAYED");
    *delay = actual_depart_time - unix_epoch_time;
    return DELAYED;
  } else {
    // Case: Flight was not delayed
    *delay = 0;
    LL_DEBUG("DEPARTED");
    return DEPARTED;
  }
}

flight_error FlightScraper::get_flight_delay(uint64_t unix_epoch_time, const char *flight, int *resp) {
  /* Invalid user input */
  if (flight == NULL || resp == NULL) {
    return INVALID;
  }

  // Build header for https request
  std::vector<string> header;
  header.push_back(AUTH_CODE);
  header.push_back(HOST);

  char _query[256];
  snprintf(_query, sizeof _query, "/json/FlightXML2/FlightInfoEx?ident=%s@%" PRIu64 "&howMany=5&offset=0 HTTP/1.1",
           flight, unix_epoch_time);

  HttpRequest httpRequest("flightxml.flightaware.com", _query, header);
  HttpsClient httpClient(httpRequest);
  flight_error ret;

  string api_response;
  try {
    HttpResponse response = httpClient.getResponse();
    api_response = response.getContent();
  } catch (const std::runtime_error &e) {
    LL_CRITICAL("%s", e.what());
    httpClient.close();
    return HTTP_ERROR;
  }

  if (api_response.empty()) {
    LL_CRITICAL("api return empty");
    return HTTP_ERROR;
  }
  try {
    ret = parse_response(api_response.c_str(), resp, unix_epoch_time);
  }
  catch (const std::exception &e) {
    LL_CRITICAL("%s", e.what());
    return INTERNAL_ERR;
  }

  return ret;
}

/*  The Data is structured as follows:
 *      0x00 - 0x20 string flight_number
 *      0x20 - 0x40 uint64 unix_epoch
 */
err_code FlightScraper::handle(const uint8_t *req, size_t data_len, int *resp_data) {
  if (data_len != 2 * 32) {
    LL_CRITICAL("Data_len %zu*32 is not 2*32", data_len / 32);
    return INVALID_PARAMS;
  }
  int delay;

  // 0x00 - 0x20 string flight_number
  char flight_number[33] = {0};
  memcpy(flight_number, req, 0x20);
  // 0x20 - 0x40 uint64 flight_time
  uint64_t unix_epoch = uint_bytes<uint64_t>(req + 0x20, 32);

  LL_INFO("flight: unix_epoch=%lld, flight_number=%s", unix_epoch, flight_number);

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
    case NOT_DEPARTED:
      *resp_data = delay;
      return NO_ERROR;
    case INTERNAL_ERR:
    default:
      return UNKNOWN_ERROR;
  }
}

err_code FlightScraper::handleEncryptedQuery(const uint8_t *data, size_t data_len, int *resp_data) {
  hexdump("encrypted_data", data, data_len);
  string _json_encoded_flight_info;
  try {
    _json_encoded_flight_info = decrypt_query(data, data_len);
    LL_INFO("decrypted flight info: %s", _json_encoded_flight_info.c_str());
  }
  catch (const DecryptionException &e) {
    LL_CRITICAL("Can't decrypt: %s", e.what());
    return INVALID_PARAMS;
  }
  catch (...) {
    LL_CRITICAL("unknown error");
    return INVALID_PARAMS;
  }

  picojson::value _flight_info_obj;
  string err_msg = picojson::parse(_flight_info_obj, _json_encoded_flight_info);
  if (!err_msg.empty() || !_flight_info_obj.is<picojson::object>()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return INVALID_PARAMS;
  }

  if (_flight_info_obj.contains("flight")
      && _flight_info_obj.get("flight").is<string>()
      && _flight_info_obj.contains("time")
      && _flight_info_obj.get("time").is<double>()) {
    string flight_id = _flight_info_obj.get("flight").get<string>();
    long timestamp = static_cast<long>(_flight_info_obj.get("time").get<double>());

    LL_INFO("querying flight info for %s@%ld", flight_id.c_str(), timestamp);

    int delay = 0;
    switch (get_flight_delay(timestamp, flight_id.c_str(), &delay)) {
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
      case NOT_DEPARTED:
        *resp_data = delay;
        return NO_ERROR;
      case INTERNAL_ERR:
      default:
        return UNKNOWN_ERROR;
    }
  }

  return INVALID_PARAMS;
}
