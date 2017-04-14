/*
 *  SSL server demonstration program using pthread for handling multiple
 *  clients.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */


#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#define mbedtls_snprintf   snprintf

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <atomic>
#include <csignal>

#include "mbedtls/ssl.h"
#include "mbedtls/net_v.h"
#include "mbedtls/net_f.h"
#include "mbedtls/error.h"
#include "Enclave_u.h"
#include "Utils.h"

#include <sgx_urts.h>
#include <thread>

using std::cerr;
using std::endl;
using std::exit;

typedef struct {
  int active;
  thread_info_t data;
  pthread_t thread;
} pthread_info_t;

#define MAX_NUM_THREADS 5

static pthread_info_t threads[MAX_NUM_THREADS];
sgx_enclave_id_t eid;

// thread function
void *ecall_handle_ssl_connection(void *data) {
  long int thread_id = pthread_self();
  thread_info_t *thread_info = (thread_info_t *) data;
  ssl_conn_handle(eid, thread_id, thread_info);

  cerr << "thread exiting for thread " << thread_id << endl;
  mbedtls_net_free(&thread_info->client_fd);
  return (NULL);
}

static int thread_create(mbedtls_net_context *client_fd) {
  int ret, i;

  for (i = 0; i < MAX_NUM_THREADS; i++) {
    if (threads[i].active == 0)
      break;

    if (threads[i].data.thread_complete == 1) {
      mbedtls_printf("  [ main ]  Cleaning up thread %d\n", i);
      pthread_join(threads[i].thread, NULL);
      memset(&threads[i], 0, sizeof(pthread_info_t));
      break;
    }
  }

  if (i == MAX_NUM_THREADS)
    return (-1);

  threads[i].active = 1;
  threads[i].data.config = NULL;
  threads[i].data.thread_complete = 0;
  memcpy(&threads[i].data.client_fd, client_fd, sizeof(mbedtls_net_context));

  if ((ret = pthread_create(&threads[i].thread, NULL, ecall_handle_ssl_connection, &threads[i].data)) != 0) {
    return (ret);
  }

  return (0);
}

std::atomic<bool> quit(false);
void exitGraceful(int) { quit.store(true); }

int main(void) {
  int ret;
  // register Ctrl-C handler
  std::signal(SIGINT, exitGraceful);

  if (0 != initialize_enclave(&eid)) {
    cerr << "failed to init enclave" << endl;
    exit(-1);
  }

  mbedtls_net_context listen_fd, client_fd;
  // initialize the object
  ssl_conn_init(eid);
  // initialize threads
  memset(threads, 0, sizeof(threads));

  mbedtls_printf("  . Bind on https://localhost:4433/ ...");
  fflush(stdout);

  if ((ret = mbedtls_net_bind(&listen_fd, NULL, "4433", MBEDTLS_NET_PROTO_TCP)) != 0) {
    mbedtls_printf(" failed\n  ! mbedtls_net_bind returned %d\n\n", ret);
    std::exit(-1);
  }

  mbedtls_printf(" ok\n");

  mbedtls_printf("  [ main ]  Waiting for a remote connection\n");

  // non-block accept
  while (true) {
    // check for Ctrl-C flag
    std::this_thread::sleep_for (std::chrono::seconds(1));
    if (quit.load()) {
      cerr << "Ctrl-C pressed. Quiting..." << endl;
      break;
    }

#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
      char error_buf[100];
      mbedtls_strerror(ret, error_buf, 100);
      mbedtls_printf("  [ main ]  Last error was: -0x%04x - %s\n", -ret, error_buf);
    }
#endif

    /*
     * 3. Wait until a client connects
     */

    if (0 != mbedtls_net_set_nonblock(&listen_fd)) {
      cerr << "can't set nonblock for the listen socket" << endl;
    }
    ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
      ret = 0;
      continue;
    } else if (ret != 0) {
      mbedtls_printf("  [ main ] failed: mbedtls_net_accept returned -0x%04x\n", ret);
      break;
    }

    mbedtls_printf("  [ main ]  ok\n");
    mbedtls_printf("  [ main ]  Creating a new thread for client %d\n", client_fd.fd);

    if ((ret = thread_create(&client_fd)) != 0) {
      mbedtls_printf("  [ main ]  failed: thread_create returned %d\n", ret);
      mbedtls_net_free(&client_fd);
      continue;
    }
    ret = 0;
  } // while (true)

  sgx_destroy_enclave(eid);
  return (ret);
}