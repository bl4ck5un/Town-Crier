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

  // debug level
  int debug_level;

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
