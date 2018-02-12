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
 * the TowCrier source code. No other rights to use TownCrier and its
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

#ifndef MBEDTLS_SGX_SSL_SERVER_THREAD_H
#define MBEDTLS_SGX_SSL_SERVER_THREAD_H

#define MBEDTLS_CONFIG_FILE "config_client.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_snprintf   snprintf
#endif

#include <stdlib.h>
#include <string.h>

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_v.h"
#include "mbedtls/net_f.h"
#include "mbedtls/error.h"

#include <stdio.h>
#include <string>
#include <sgx_thread.h>

#include "../common/ssl_context.h"

using std::string;

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "<h2>mbed TLS Test Server</h2>\r\n" \
    "<p>Successful connection using: %s</p>\r\n"

class TLSConnectionHandler {
 private:
  /*
   * static members
   */
  const static string pers;
  static sgx_thread_mutex_t mutex;

  /*
   * global server state
   */
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_config conf;
  mbedtls_x509_crt srvcert;
  mbedtls_x509_crt cachain;
  mbedtls_pk_context pkey;

  /*
   * configuration
   */
  unsigned int debug_level;

  /*
   * debug callback
   */
  static void mydebug(void *ctx, int level,
               const char *file, int line,
               const char *str);

 public:
  TLSConnectionHandler();
  TLSConnectionHandler(unsigned int debug_level) : debug_level(debug_level) {
    TLSConnectionHandler();
  }
  ~TLSConnectionHandler();
  void handle(long int, thread_info_t *);
};

#endif //MBEDTLS_SGX_SSL_SERVER_THREAD_H
