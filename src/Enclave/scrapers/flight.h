/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially
 * docketed at the Center for Technology Licensing at Cornell University
 * as D-7364, developed through research conducted at Cornell University,
 * and its associated copyrights solely for educational, research and
 * non-profit purposes without fee is hereby granted, provided that the
 * user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling
 * the TownCrier source code. No other rights to use TownCrier and its
 * associated copyrights for any other purpose are granted herein,
 * whether commercial or non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial
 * products or use TownCrier and its associated copyrights for commercial
 * purposes must contact the Center for Technology Licensing at Cornell
 * University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
 * ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
 * commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
 * ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
 * UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
 * REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
 * OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
 * OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
 * PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science
 * Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
 * CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
 * Google Faculty Research Awards, and a VMWare Research Award.
 */

#ifndef SRC_ENCLAVE_SCRAPERS_FLIGHT_H_
#define SRC_ENCLAVE_SCRAPERS_FLIGHT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "Scraper.h"
#include "debug.h"
#include "../log.h"
#include "utils.h"
#include "Constants.h"
#include "external/slre.h"
#include "tls_client.h"

/* Define flight scraper specific errors */
enum flight_error {
  INVALID = 0,          /* Invalid Parameters passed to the function*/
  DEPARTED,           /* The Flight has departed with no delays */
  DELAYED,            /* The flight is delayed */
  CANCELLED,          /* The flight has been cancelled */
  NOT_DEPARTED,       /* The flight has not departed */
  NOT_FOUND,          /* The flight was not found */
  HTTP_ERROR,         /* HTTP request failed */
  INTERNAL_ERR,
};

class FlightScraper : Scraper {
 private:
  static const int MAX_DELAY_MIN = 30;
  static const char *HOST;
  static const char *AUTH_CODE;
 public:
  err_code handle(const uint8_t *req, size_t data_len, int *resp_data);
  err_code handleEncryptedQuery(const uint8_t *data, size_t data_len, int *resp_data);
  flight_error get_flight_delay(uint64_t unix_epoch_time, const char *flight, int *resp);
 private:
  flight_error parse_response(const string &resp, int *delay, uint64_t unix_epoch_time);
};

#endif  // SRC_ENCLAVE_SCRAPERS_FLIGHT_H_
