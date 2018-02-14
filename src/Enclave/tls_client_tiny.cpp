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

#include "tls_client.h"
#include "log.h"
#include "Enclave_t.h"
#include "debug.h"
#include "Constants.h"
#include "ca_bundle.h"

#include <algorithm>

#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "external/tinyhttp/http.h"

// directly embed the source here
extern "C" {
#include "external/tinyhttp/http.c"
#include "external/tinyhttp/header.c"
#include "external/tinyhttp/chunk.c"
}

#define MIN(x, y) (x < y ? x : y)

#if !defined(MBEDTLS_CONFIG_FILE)

#include "mbedtls/config.h"

#else
#include MBEDTLS_CONFIG_FILE
#endif

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <exception>
#include <vector>

using namespace std;

static const string printableRequest(const string &request) {
  std::string res;
  for (int i = 0; i < request.length(); ++i) {
    switch (request[i]) {
      case '\r':
        res += "\\r";
        break;
      case '\n':
        res += "\\n";
        break;
      default:
        res += request[i];
    }
  }
  return res;
}

// Response data/funcs
struct HttpResponseTiny {
  std::vector<char> body;
  int code;
};

static void *response_realloc(void *opaque, void *ptr, int size) {
  (void) opaque;
  return realloc(ptr, size);
}

static void response_body(void *opaque, const char *data, int size) {
  HttpResponseTiny *response = (HttpResponseTiny *) opaque;
  response->body.insert(response->body.end(), data, data + size);
}

static void response_header(void *opaque,
                            const char *ckey,
                            int nkey,
                            const char *cvalue,
                            int nvalue) {
  (void) opaque;
  LL_TRACE("(%d, %d) %s: %s", nkey, nvalue, ckey, cvalue);
}

static void response_code(void *opaque, int code) {
  HttpResponseTiny *response = (HttpResponseTiny *) opaque;
  response->code = code;
}

static const http_funcs responseFuncs = {
    response_realloc,
    response_body,
    response_header,
    response_code,
};

static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str) {
  const char *p, *basename;
  (void) (ctx);

  /* Extract basename from file */
  for (p = basename = file; *p != '\0'; p++)
    if (*p == '/' || *p == '\\')
      basename = p + 1;

  mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
}

#if defined(MBEDTLS_X509_CRT_PARSE_C)

/*
 * Enabled if debug_level > 1 in code below
 */
static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
  char buf[1024];
  ((void) data);

  LL_DEBUG("\nVerify requested for (Depth %d):", depth);
  mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
  LL_DEBUG("%s", buf);

  if ((*flags) == 0) {
    LL_DEBUG("  This certificate has no flags");
  } else {
    mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", *flags);
    LL_DEBUG("%s", buf);
  }

  return (0);
}

#endif /* MBEDTLS_X509_CRT_PARSE_C */

HttpsClient::HttpsClient(HttpRequest &httpRequest) : httpRequest(httpRequest) {
  int ret = 0;
  mbedtls_net_init(&server_fd);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  memset(&saved_session, 0, sizeof(mbedtls_ssl_session));
  mbedtls_ctr_drbg_init(&ctr_drbg);
#if defined(MBEDTLS_X509_CRT_PARSE_C)
  mbedtls_x509_crt_init(&cacert);
  mbedtls_x509_crt_init(&clicert);
  mbedtls_pk_init(&pkey);
#endif

#if defined(MBEDTLS_DEBUG_C)
  mbedtls_debug_set_threshold(0);
#endif

  mbedtls_entropy_init(&entropy);

  /*
   * 0. Initialize the RNG and the session data
   */
  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char *) pers,
                                   strlen(pers))) != 0) {
    LL_CRITICAL(" mbedtls_ctr_drbg_seed returned -%#x", -ret);
    throw std::runtime_error("mbedtls_ctr_drbg_seed failed");
  }

  /*
   * 1. Load the trusted CA
   */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
  ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *) mozilla_ca_bundle, sizeof(mozilla_ca_bundle));
  if (ret < 0) {
    throw std::runtime_error("mbedtls_x509_crt_parse failed");
  }

#endif /* MBEDTLS_X509_CRT_PARSE_C */

}

string HttpsClient::buildRequestMessage() {
  string requestMessage;
  requestMessage += string("GET ") + httpRequest.getUrl();
  if (httpRequest.getIsHttp11() && requestMessage.find("HTTP/1.1") == string::npos) {
    requestMessage += " HTTP/1.1";
  }
  requestMessage += string("\r\n");

  for (vector<string>::const_iterator it = httpRequest.getHeaders().begin();
       it != httpRequest.getHeaders().end(); it++) {
    requestMessage += (*it) + "\r\n";
  }

  requestMessage += "Accept: text/html\r\n";
  if (httpRequest.getIsHttp11() && requestMessage.find("Host:") == string::npos) {
    requestMessage += "Host: " + httpRequest.getHost() + "\r\n";
  }

  requestMessage += HttpsClient::GET_END;

  return requestMessage;
}

void HttpsClient::sendRequest() {
  string requestMessage = buildRequestMessage();

#ifdef HEXDUMP_TLS_TRANSCRIPT
  dump_buf("Request: ", (const unsigned char *) requestMessage.c_str(), requestMessage.length());
#else
  LL_DEBUG("Request: %s", printableRequest(requestMessage).c_str());
#endif

  for (int written = 0, frags = 0; written < requestMessage.size(); written += ret, frags++) {
    while ((ret = mbedtls_ssl_write(&ssl,
                                    reinterpret_cast<const unsigned char *>(requestMessage.c_str()) + written,
                                    requestMessage.size() - written)) <= 0) {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
          ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
        mbedtls_printf("  mbedtls_ssl_write returned -%#x", -ret);
        throw runtime_error("mbedtls_ssl_write");
      }
    }
  }
}

HttpResponse HttpsClient::getResponse() {
  ret = 0;
  /*
   * 2. Start the connection
   */
  LL_TRACE("connecting over TCP: %s:%s...", httpRequest.getHost().c_str(), httpRequest.getPort().c_str());

  if ((ret = mbedtls_net_connect(&server_fd, httpRequest.getHost().c_str(), httpRequest.getPort().c_str(),
                                 MBEDTLS_NET_PROTO_TCP)) != 0) {
    throw std::runtime_error("mbedtls_net_connect returned");
  }

  ret = mbedtls_net_set_block(&server_fd);
  if (ret != 0) {
    throw std::runtime_error("net_set_(non)block()");
  }

  /*
   * 3. Setup stuff
   */
  LL_TRACE("Setting up the SSL/TLS structure...");

  if ((ret = mbedtls_ssl_config_defaults(&conf,
                                         MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    throw runtime_error("mbedtls_ssl_config_defaults");
  }

#if defined(TRACE_TLS_CLIENT)
  mbedtls_ssl_conf_verify(&conf, my_verify, NULL);
#endif

#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH)
  if ((ret = mbedtls_ssl_conf_max_frag_len(&conf, MBEDTLS_SSL_MAX_FRAG_LEN_NONE)) != 0) {
    throw runtime_error("mbedtls_ssl_conf_max_frag_len");
  }
#endif

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
  mbedtls_ssl_conf_dbg(&conf, my_debug, NULL);

  mbedtls_ssl_conf_read_timeout(&conf, 0);
  mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);

#if defined(MBEDTLS_SSL_SESSION_TICKETS)
  mbedtls_ssl_conf_session_tickets(&conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
#endif

#if defined(MBEDTLS_SSL_RENEGOTIATION)
  mbedtls_ssl_conf_renegotiation(&conf, MBEDTLS_SSL_RENEGOTIATION_DISABLED);
#endif

  if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
    throw runtime_error("mbedtls_ssl_setup");
  }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
  if ((ret = mbedtls_ssl_set_hostname(&ssl, httpRequest.getHost().c_str())) != 0) {
    throw runtime_error("mbedtls_ssl_set_hostname");
  }
#endif

  mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  /*
   * 4. Handshake
   */
  LL_TRACE("Performing the SSL/TLS handshake");

  while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {

#if defined(MBEDTLS_X509_CRT_PARSE_C)
      LL_TRACE("Verifying peer X.509 certificate...");
      if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
        LL_CRITICAL("X.509 certificate failed to verify");
        char temp_buf[1024];
        if (mbedtls_ssl_get_peer_cert(&ssl) != NULL) {
          LL_CRITICAL("Peer certificate information");
          mbedtls_x509_crt_info((char *) temp_buf, sizeof(temp_buf) - 1, "|-", mbedtls_ssl_get_peer_cert(&ssl));
          mbedtls_printf("%s\n", temp_buf);
        } else {
          LL_CRITICAL("mbedtls_ssl_get_peer_cert returns NULL");
        }
      } else {
        LL_TRACE("X.509 Verifies");
      }
#endif /* MBEDTLS_X509_CRT_PARSE_C */
      if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
        LL_CRITICAL("Unable to verify the server's certificate.");
      }

      throw runtime_error("mbedtls_ssl_handshake failed.");
    }
  }

  LL_TRACE("Hand shake succeeds: [%s, %s]", mbedtls_ssl_get_version(&ssl), mbedtls_ssl_get_ciphersuite(&ssl));

  if ((ret = mbedtls_ssl_get_record_expansion(&ssl)) >= 0) {
    LL_TRACE("Record expansion is [%d]", ret);
  } else
    LL_TRACE("Record expansion is [unknown (compression)]");

#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH)
  LL_TRACE("Maximum fragment length is [%u]",
           (unsigned int) mbedtls_ssl_get_max_frag_len(&ssl));
#endif

  sendRequest();

  HttpResponseTiny response;
  response.code = 0;

  http_roundtripper rt;
  http_init(&rt, responseFuncs, &response);

  unsigned char buffer[4096];
  bool http_need_more = true;
  while (http_need_more) {
    /*
    return the number of bytes read, or 0 for EOF, or
      MBEDTLS_ERR_SSL_WANT_READ or MBEDTLS_ERR_SSL_WANT_WRITE, or
      MBEDTLS_ERR_SSL_CLIENT_RECONNECT (see below), or another negative
      error code.
    */
    const unsigned char *data = buffer;
    int n_data = mbedtls_ssl_read(&ssl, buffer, sizeof(buffer));

    LL_TRACE("mbedtls_ssl_read returns %d (Content-Length=%d)", n_data, rt.contentlength);

    if (n_data == MBEDTLS_ERR_SSL_WANT_READ ||
        n_data == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;

    // EOF reached
    if (n_data == 0 && rt.contentlength == -1) {
      LL_DEBUG("eof reached");
      break;
    }

    if (n_data < 0) {
      ret = n_data;
      switch (n_data) {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          LL_CRITICAL(" connection was closed gracefully");
          http_free(&rt);
          throw runtime_error("connection was closed gracefully");
        case MBEDTLS_ERR_NET_CONN_RESET:
          LL_CRITICAL(" connection was reset by peer");
          http_free(&rt);
          throw runtime_error("connected reset");
        default:
          LL_CRITICAL(" mbedtls_ssl_read returned 0x%x", n_data);
          http_free(&rt);
          throw runtime_error("mbedtls_ssl_read returned non-sense");
      }
    }
    while (http_need_more && n_data) {
      int read;
      http_need_more = (bool) http_data(&rt, (const char *) data, n_data, &read);
      n_data -= read;
      data += read;
    }
  } // while (http_need_more)


  if (http_iserror(&rt)) {
    http_free(&rt);
    throw runtime_error("Error parsing HTTP data");
  }

  LL_DEBUG("HTTP response len=%zu", response.body.size());

  string content(response.body.begin(), response.body.end());

  HttpResponse resp(response.code, "", content);

  LL_TRACE("Response body (len=%zu):\n%s",
           content.length(),
           content.length() == 0 ? "empty" : content.c_str());

  http_free(&rt);
  return resp;
}

void HttpsClient::close() {
  do ret = mbedtls_ssl_close_notify(&ssl);
  while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
  ret = 0;

  LL_TRACE("closed %s:%s", httpRequest.getHost().c_str(), httpRequest.getPort().c_str());
}

string HttpsClient::getError() {
#ifdef MBEDTLS_ERROR_C
  if (ret != 0) {
    char error_buf[100];
    mbedtls_strerror(ret, error_buf, sizeof error_buf);
    return string(error_buf);
  }
#endif
  return "";
}

HttpsClient::~HttpsClient() {
  mbedtls_net_free(&server_fd);
#if defined(MBEDTLS_X509_CRT_PARSE_C)
  mbedtls_x509_crt_free(&clicert);
  mbedtls_x509_crt_free(&cacert);
  mbedtls_pk_free(&pkey);
#endif
  mbedtls_ssl_session_free(&saved_session);
  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
}

const string HttpsClient::GET_END = "\r\n";
const char *HttpsClient::pers = "Town-Crier";
