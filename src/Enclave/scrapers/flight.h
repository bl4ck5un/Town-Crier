#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "Scraper.h"
#include "Debug.h"
#include "Log.h"
#include "utils.h"
#include "../../Common/Constants.h"
#include "../external/slre.h"
#include "tls_client.h"

/* Define flight scraper specific errors */
enum flight_error {
  // TODO: oscar: start 1?
  INVALID = 0,          /* Invalid Parameters passed to the function*/
  DEPARTED,           /* The Flight has departed with no delays */
  DELAYED,            /* The flight is delayed */
  CANCELLED,          /* The flight has been cancelled */
  NOT_DEPARTED,       /* The flight has not departed */
  NOT_FOUND,          /* The flight was not found */
  HTTP_ERROR,         /* HTTP request failed */
};

class FlightScraper : Scraper {
 public:
  err_code handler(uint8_t *req, int data_len, int *resp_data);
  flight_error parse_response(const char *resp, int *delay, uint64_t unix_epoch_time);
  flight_error get_flight_delay(uint64_t unix_epoch_time, const char *flight, int *resp);
  std::string uint64_to_string(uint64_t value);
};

#endif