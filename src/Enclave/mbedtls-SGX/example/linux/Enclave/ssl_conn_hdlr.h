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
