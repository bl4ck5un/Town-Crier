#include <mbedtls/bignum.h>
#include "Log.h"
#include "Debug.h"
#include "eth_ecdsa.h"

#define SECKEY_TEST "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"

extern "C" int keygen_test();
int keygen_test() {
  mbedtls_mpi seckey;
  mbedtls_mpi_init(&seckey);

  int ret;
  ret = mbedtls_mpi_read_string(&seckey, 16, SECKEY_TEST);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_mpi_read_string returned %d\n", ret);
    return -1;
  }

  unsigned char pubkey[64];
  unsigned char address[20];

  ret = __ecdsa_keygen(&seckey, pubkey, address);
  dump_buf("publicKey:", pubkey, 64);
  dump_buf("address:", address, 20);
  printf_sgx("Want: 0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997\n");

  return ret;
}