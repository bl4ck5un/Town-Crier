//
// Created by fanz on 4/7/17.
//

#ifndef MBEDTLS_SGX_SSL_CONTEXT_H
#define MBEDTLS_SGX_SSL_CONTEXT_H

#include "mbedtls/ssl.h"
#include "mbedtls/net_v.h"

typedef struct {
  mbedtls_net_context client_fd;
  int thread_complete;
  const mbedtls_ssl_config *config;
} thread_info_t;

#endif //MBEDTLS_SGX_SSL_CONTEXT_H
