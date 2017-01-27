#include <iostream>

#include "Converter.h"
#include "Enclave_u.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  sgx_enclave_id_t eid;
  sgx_status_t st;
  int ret;

  ret = initialize_tc_enclave(&eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_NOTICE("enclave %lu created", eid);
  }

  unsigned char pubkey[64];
  unsigned char address[20];
  st = ecdsa_keygen(eid, &ret, pubkey, address);
  if (st != SGX_SUCCESS) {
    LL_CRITICAL("ecall failed");
  }
  if (ret != 0) {
    LL_CRITICAL("keygen_test returns %d", ret);
  }
  std::cout << "PublicKey: " << bufferToHex(pubkey, sizeof pubkey, true) << std::endl;
  std::cout << "Address: " << bufferToHex(address, sizeof address, true) << std::endl;

  sgx_destroy_enclave(eid);
  LL_CRITICAL("Info: all enclave closed successfully.");
}
