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
#include "Log.h"
#include "Enclave_t.h"
#include "debug.h"
#include "external/http_parser.h"
#include "Constants.h"
#include "ca_bundle.h"

#include <algorithm>

#define MIN(x,y) (x < y ? x : y)

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
#include <stlport/type_traits>

using namespace std;

static const string printableRequest(const string &request) {
  std::string res;
  for (int i = 0; i < request.length(); ++i) {
    switch (request[i]) {
      case '\r':res += "\\r";
        break;
      case '\n':res += "\\n";
        break;
      default:res += request[i];
    }
  }
  return res;
}

typedef struct {
  ReceivingBuffer *buffer;
  uint8_t eof;
  size_t header_length;
} cb_data_t;

int cb_on_message_complete(http_parser *parser) {
  LL_TRACE("message_complete called");
  cb_data_t *d = (cb_data_t *) parser->data;
  d->eof = 1;
  return 0;
}

int cb_on_body(http_parser *parser, const char *at, size_t len) {
  LL_TRACE("On body called with at=%p and len=%zu", at, len);
  cb_data_t *p = (cb_data_t *) parser->data;

  return 0;
}

int cb_on_header_complete(http_parser *parser) {
  cb_data_t *p = (cb_data_t *) parser->data;
  p->header_length = parser->nread;
  LL_TRACE("header_complete called after reading %zu", p->header_length);

  return 0;
}

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

/*
 * Test recv/send functions that make sure each try returns
 * WANT_READ/WANT_WRITE at least once before sucesseding
 */
static int my_recv(void *ctx, unsigned char *buf, size_t len) {
  static int first_try = 1;
  int ret;

  if (first_try) {
    first_try = 0;
    return (MBEDTLS_ERR_SSL_WANT_READ);
  }

  ret = mbedtls_net_recv(ctx, buf, len);
  if (ret != MBEDTLS_ERR_SSL_WANT_READ)
    first_try = 1; /* Next call will be a new operation */
  return (ret);
}

static int my_send(void *ctx, const unsigned char *buf, size_t len) {
  static int first_try = 1;
  int ret;

  if (first_try) {
    first_try = 0;
    return (MBEDTLS_ERR_SSL_WANT_WRITE);
  }

  ret = mbedtls_net_send(ctx, buf, len);
  if (ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    first_try = 1; /* Next call will be a new operation */
  return (ret);
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

  size_t n_parsed = 0;

  http_parser parser;
  http_parser_init(&parser, HTTP_RESPONSE);

  http_parser_settings settings;
  http_parser_settings_init(&settings);

  cb_data_t cb_data;
  cb_data.eof = 0;
  cb_data.header_length = 0;
  cb_data.buffer = &buf;
  parser.data = &cb_data;
  settings.on_message_complete = cb_on_message_complete;
  settings.on_headers_complete = cb_on_header_complete;

  while (true) {
    /*
    return the number of bytes read, or 0 for EOF, or
    MBEDTLS_ERR_SSL_WANT_READ or MBEDTLS_ERR_SSL_WANT_WRITE, or
    MBEDTLS_ERR_SSL_CLIENT_RECONNECT (see below), or another negative
    error code.
    */
    ret = mbedtls_ssl_read(&ssl, buf.buf + buf.length, buf.cap - buf.length);

    LL_TRACE("mbedtls_ssl_read returns %d", ret);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
        ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;

    if (ret < 0) {
      switch (ret) {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:LL_CRITICAL(" connection was closed gracefully");
          throw runtime_error("connection was closed gracefully");
        case MBEDTLS_ERR_NET_CONN_RESET:LL_CRITICAL(" connection was reset by peer");
          throw runtime_error("connected reset");
        default:LL_CRITICAL(" mbedtls_ssl_read returned -0x%x", -ret);
          throw runtime_error("mbedtls_ssl_read returned non-sense");
      }
    } else {
      if (ret == 0) {
        break;
      }

      LL_TRACE("%d bytes received.", ret);
      buf.length += ret;

      n_parsed = http_parser_execute(&parser, &settings, reinterpret_cast<const char *>(buf.buf),
                                     (size_t) ret);
      if (parser.upgrade) {
        ret = ERR_ENCLAVE_SSL_CLIENT;
        throw runtime_error("upgrade not supported");
      } else if (n_parsed != ret) {
        string _partial_buffer(buf.buf, buf.buf + buf.length);

        // TODO: nasty hack: a workaround to http_parser bug
        if (httpRequest.getHost() == "ichart.yahoo.com") {
          // this is actually okay. Pretend everything is fine and set EOF
          cb_data.eof = 1;
          // remove the trailing \r\n
          if (_partial_buffer.compare(_partial_buffer.length() - 2, 2, "\r\n")) {
            _partial_buffer.erase(_partial_buffer.end()-1, _partial_buffer.end());
          }
          return HttpResponse(200, "", _partial_buffer);
        }
        LL_CRITICAL("Error: received %d bytes and parsed %zu of them", ret, n_parsed);
        LL_CRITICAL("Error: %s", http_errno_name(HTTP_PARSER_ERRNO((&parser))));

        printf_sgx(_partial_buffer.c_str());
        ret = ERR_ENCLAVE_SSL_CLIENT;
        throw runtime_error("received bytes are can not be fully parsed: " + string(http_errno_name(HTTP_PARSER_ERRNO((&parser)))));
      }

      if (cb_data.eof == 1) {
        LL_TRACE("EOF");
        if (parser.status_code != 200) {
          LL_CRITICAL("status code %d", parser.status_code);
        } else
          LL_LOG("status code %d", parser.status_code);
        break;
      }
    }
  } // while (true)

  if (cb_data.eof == 0 && buf.length == buf.cap) {
    LL_CRITICAL("receiving buffer (%zu bytes) is not big enough", buf.cap);
  }

  string response_headers(reinterpret_cast<const char *>(buf.buf),
                          cb_data.header_length == 0 ? buf.length : cb_data.header_length);

  string content((const char *) buf.buf + cb_data.header_length, buf.length - cb_data.header_length);
  HttpResponse resp(parser.status_code, response_headers, content);

  LL_DEBUG("\nResponse header:\n%s",
           response_headers.length() == 0 ? "empty" : response_headers.c_str());
  LL_DEBUG("\nResponse body (len=%zu):\n%s",
           content.length(),
           content.length() == 0 ? "empty" : content.substr(0, HttpsClient::responseLogLimit).c_str());

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
    mbedtls_strerror(ret, error_buf, 100);
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
