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

#include <stdio.h>
#include <stdlib.h>

#include "Enclave_t.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_v.h"
#include "mbedtls/net_f.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#ifndef SSL_CLIENT_H
#define SSL_CLIENT_H

#include <string>
#include <vector>

using namespace std;

class HttpRequest {
 private:
  const string host;
  const string port;
  const string url;
  const vector <string> headers;
  const bool isHttp11;
 public:
  HttpRequest(const string &host, const string &url) :
      host(host), port("443"), url(url), isHttp11(false) {};

  HttpRequest(const string &host, const string &url, bool isHttp11) :
      host(host), port("443"), url(url), isHttp11(isHttp11) {};

  HttpRequest(const string &host, const string &url, const vector <string> &headers) :
      host(host), port("443"), url(url), headers(headers), isHttp11(false) {};

  HttpRequest(const string &host, const string &url, const vector <string> &headers, bool isHttp11) :
      host(host), port("443"), url(url), headers(headers), isHttp11(isHttp11) {};

  HttpRequest(const string &host, const string &port, const string &url, const vector <string> &headers, bool isHttp11)
      :
      host(host), port(port), url(url), headers(headers), isHttp11(isHttp11) {};

  const string &getHost() const {
    return host;
  }

  const string &getPort() const {
    return port;
  }

  const string &getUrl() const {
    return url;
  }

  const vector <string> &getHeaders() const {
    return headers;
  }

  const bool getIsHttp11() const {
    return isHttp11;
  }
};

class HttpResponse {
 private:
  const int statusCode;
  const string headers;
  const string content;
 public:
  HttpResponse(int status_code, const string headers, const string content) : statusCode(status_code),
                                                                              headers(headers), content(content) {}

  const int getStatusCode() const {
    return statusCode;
  }

  const string &getHeaders() const {
    return headers;
  }

  const string &getContent() const {
    return content;
  }
};

class ReceivingBuffer {
 public:
  unsigned char *buf;
  const static size_t cap = 2 * 1024 * 1024; // 2MB
  size_t length;

  ReceivingBuffer() {
    length = 0;
    buf = (unsigned char *) malloc(cap);
    if (buf == NULL)
      throw bad_alloc();
  }

  ~ReceivingBuffer() {
    length = 0;
    free(buf);
  }
};

class HttpsClient {
 private:
  HttpRequest &httpRequest;

  // errno
  int ret;

  // how many chars to log
  static const int responseLogLimit = 200;

  static const char *pers;
  static const string GET_END;

  // resources
  ReceivingBuffer buf;
  mbedtls_net_context server_fd;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ssl_session saved_session;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
  uint32_t flags;
  mbedtls_x509_crt cacert;
  mbedtls_x509_crt clicert;
  mbedtls_pk_context pkey;
#endif

 private:
  string buildRequestMessage();
  void sendRequest();

 public:
  HttpsClient(HttpRequest &httpRequest);
  HttpResponse getResponse();
  void close();
  string getError();
  ~HttpsClient();
};

#endif
