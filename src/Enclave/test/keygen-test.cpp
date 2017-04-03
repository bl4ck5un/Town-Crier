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

  if ((ret = __ecdsa_sec_to_pub(&seckey, pubkey, address))) {
    LL_CRITICAL("__ecdsa_sec_to_pub returned %d", ret);
    return -1;
  }
  return memcmp(address, "\x89\xb4\x4e\x4d\x3c\x81\xed\xe0\x5d\x0f\x5d\xe8\xd1\xa6\x8f\x75\x4d\x73\xd9\x97", 20);
}