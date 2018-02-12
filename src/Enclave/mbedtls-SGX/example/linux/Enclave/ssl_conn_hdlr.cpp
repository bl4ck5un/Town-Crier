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
// the TowCrier source code. No other rights to use TownCrier and its
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

#include "ssl_conn_hdlr.h"

#include <exception>
#include <mbedtls/net_v.h>
#include "mbedtls/net_f.h"
#include "mbedtls/debug.h"

TLSConnectionHandler::TLSConnectionHandler() {
  int ret;

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
  unsigned char alloc_buf[100000];
#endif
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_context cache;
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
  mbedtls_memory_buffer_alloc_init( alloc_buf, sizeof(alloc_buf) );
#endif

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_init( &cache );
#endif

  mbedtls_x509_crt_init(&srvcert);
  mbedtls_x509_crt_init(&cachain);

  mbedtls_ssl_config_init(&conf);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  /*
   * We use only a single entropy source that is used in all the threads.
   */
  mbedtls_entropy_init(&entropy);

  /*
   * 1. Load the certificates and private RSA key
   */
  mbedtls_printf("\n  . Loading the server cert. and key...");

  /*
   * This demonstration program uses embedded test certificates.
   * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
   * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
   */
  ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_srv_crt,
                               mbedtls_test_srv_crt_len);
  if (ret != 0) {
    mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
    throw std::runtime_error("");
  }

  ret = mbedtls_x509_crt_parse(&cachain, (const unsigned char *) mbedtls_test_cas_pem,
                               mbedtls_test_cas_pem_len);
  if (ret != 0) {
    mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
    throw std::runtime_error("");
  }

  mbedtls_pk_init(&pkey);
  ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key,
                             mbedtls_test_srv_key_len, NULL, 0);
  if (ret != 0) {
    mbedtls_printf(" failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
    throw std::runtime_error("");
  }

  mbedtls_printf(" ok\n");

  /*
   * 1b. Seed the random number generator
   */
  mbedtls_printf("  . Seeding the random number generator...");

  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char *) pers.c_str(),
                                   pers.length())) != 0) {
    mbedtls_printf(" failed: mbedtls_ctr_drbg_seed returned -0x%04x\n",
                   -ret);
    throw std::runtime_error("");
  }

  mbedtls_printf(" ok\n");

  /*
   * 1c. Prepare SSL configuration
   */
  mbedtls_printf("  . Setting up the SSL data....");

  if ((ret = mbedtls_ssl_config_defaults(&conf,
                                         MBEDTLS_SSL_IS_SERVER,
                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    mbedtls_printf(" failed: mbedtls_ssl_config_defaults returned -0x%04x\n",
                   -ret);
    throw std::runtime_error("");
  }

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

  /*
   * setup debug
   */
  mbedtls_ssl_conf_dbg(&conf, mydebug, NULL);
  // if debug_level is not set (could be set via other constructors), set it to 0
  if (debug_level < 0) {
    debug_level = 0;
  }
  mbedtls_debug_set_threshold(debug_level);

  /* mbedtls_ssl_cache_get() and mbedtls_ssl_cache_set() are thread-safe if
   * MBEDTLS_THREADING_C is set.
   */
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_conf_session_cache( &conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set );
#endif

  mbedtls_ssl_conf_ca_chain(&conf, &cachain, NULL);
  if ((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0) {
    mbedtls_printf(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
    throw std::runtime_error("");
  }

  mbedtls_printf(" ok\n");
}

TLSConnectionHandler::~TLSConnectionHandler() {
  mbedtls_x509_crt_free(&srvcert);
  mbedtls_pk_free(&pkey);
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_free( &cache );
#endif
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
  mbedtls_ssl_config_free(&conf);

  sgx_thread_mutex_destroy(&mutex);

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
  mbedtls_memory_buffer_alloc_free();
#endif

#if defined(_WIN32)
  mbedtls_printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif
}

void TLSConnectionHandler::handle(long int thread_id, thread_info_t *thread_info) {
  int ret, len;
  mbedtls_net_context *client_fd = &thread_info->client_fd;
  unsigned char buf[1024];
  mbedtls_ssl_context ssl;

  // thread local data
  mbedtls_ssl_config conf;
  memcpy(&conf, &this->conf, sizeof(mbedtls_ssl_config));
  thread_info->config = &conf;
  thread_info->thread_complete = 0;

  /* Make sure memory references are valid */
  mbedtls_ssl_init(&ssl);

  mbedtls_printf("  [ #%ld ]  Setting up SSL/TLS data\n", thread_id);

  /*
   * 4. Get the SSL context ready
   */
  if ((ret = mbedtls_ssl_setup(&ssl, thread_info->config)) != 0) {
    mbedtls_printf("  [ #%ld ]  failed: mbedtls_ssl_setup returned -0x%04x\n",
                   thread_id, -ret);
    goto thread_exit;
  }

  mbedtls_printf("client_fd is %d\n", client_fd->fd);
  mbedtls_ssl_set_bio(&ssl, client_fd, mbedtls_net_send, mbedtls_sgx_net_recv, NULL);

  /*
   * 5. Handshake
   */
  mbedtls_printf("  [ #%ld ]  Performing the SSL/TLS handshake\n", thread_id);

  while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      mbedtls_printf("  [ #%ld ]  failed: mbedtls_ssl_handshake returned -0x%04x\n",
                     thread_id, -ret);
      goto thread_exit;
    }
  }

  mbedtls_printf("  [ #%ld ]  ok\n", thread_id);

  /*
   * 6. Read the HTTP Request
   */
  mbedtls_printf("  [ #%ld ]  < Read from client\n", thread_id);

  do {
    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));
    ret = mbedtls_ssl_read(&ssl, buf, len);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;

    if (ret <= 0) {
      switch (ret) {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          mbedtls_printf("  [ #%ld ]  connection was closed gracefully\n",
                         thread_id);
          goto thread_exit;

        case MBEDTLS_ERR_NET_CONN_RESET:
          mbedtls_printf("  [ #%ld ]  connection was reset by peer\n",
                         thread_id);
          goto thread_exit;

        default:
          mbedtls_printf("  [ #%ld ]  mbedtls_ssl_read returned -0x%04x\n",
                         thread_id, -ret);
          goto thread_exit;
      }
    }

    len = ret;
    mbedtls_printf("  [ #%ld ]  %d bytes read\n=====\n%s\n=====\n",
                   thread_id, len, (char *) buf);

    if (ret > 0)
      break;
  } while (1);

  /*
   * 7. Write the 200 Response
   */
  mbedtls_printf("  [ #%ld ]  > Write to client:\n", thread_id);

  len = snprintf((char *) buf, sizeof buf, HTTP_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));

  while ((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
    if (ret == MBEDTLS_ERR_NET_CONN_RESET) {
      mbedtls_printf("  [ #%ld ]  failed: peer closed the connection\n",
                     thread_id);
      goto thread_exit;
    }

    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      mbedtls_printf("  [ #%ld ]  failed: mbedtls_ssl_write returned -0x%04x\n",
                     thread_id, ret);
      goto thread_exit;
    }
  }

  len = ret;
  mbedtls_printf("  [ #%ld ]  %d bytes written\n=====\n%s\n=====\n",
                 thread_id, len, (char *) buf);

  mbedtls_printf("  [ #%ld ]  . Closing the connection...", thread_id);

  while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
        ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      mbedtls_printf("  [ #%ld ]  failed: mbedtls_ssl_close_notify returned -0x%04x\n",
                     thread_id, ret);
      goto thread_exit;
    }
  }

  mbedtls_printf(" ok\n");

  ret = 0;

  thread_exit:

#ifdef MBEDTLS_ERROR_C
  if (ret != 0) {
    char error_buf[100];
    mbedtls_strerror(ret, error_buf, 100);
    mbedtls_printf("  [ #%ld ]  Last error was: -0x%04x - %s\n\n",
                   thread_id, -ret, error_buf);
  }
#endif

  mbedtls_ssl_free(&ssl);

  thread_info->config = NULL;
  thread_info->thread_complete = 1;
}

const string TLSConnectionHandler::pers = "ssl_pthread_server";
sgx_thread_mutex_t TLSConnectionHandler::mutex = SGX_THREAD_MUTEX_INITIALIZER;

void TLSConnectionHandler::mydebug(void *ctx, int level,
                                   const char *file, int line,
                                   const char *str) {
  (void) ctx;
  (void) level;
  long int thread_id = 0;
  sgx_thread_mutex_lock(&mutex);

  mbedtls_printf("%s:%04d: [ #%ld ] %s",
                 file, line, thread_id, str);

  sgx_thread_mutex_unlock(&mutex);
}