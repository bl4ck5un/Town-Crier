#include <iostream>

#include "Converter.h"
#include "Enclave_u.h"
#include "utils.h"
#include "macros.h"
#include "sgx_error.h"

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

  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];
  size_t buffer_used = 0;
  st = ecdsa_keygen_seal(eid, &ret, secret_sealed, &buffer_used, pubkey, address);
  if (st != SGX_SUCCESS) {
    LL_CRITICAL("ecall failed");
    print_error_message(st);
  }
  if (ret != 0) {
    LL_CRITICAL("ecdsa_keygen_seal returns %d", ret);
  }
  std::cout << "Sealed Secret: " << bufferToHex(secret_sealed, buffer_used, true) << std::endl;
  std::cout << "Sealed length: " << buffer_used << std::endl;
  std::cout << "PublicKey: " << bufferToHex(pubkey, sizeof pubkey, true) << std::endl;
  std::cout << "Address: " << bufferToHex(address, sizeof address, true) << std::endl;

  memset(pubkey, 0, sizeof pubkey);
  memset(address, 0, sizeof address);
  ecdsa_keygen_unseal(eid, &ret, (sgx_sealed_data_t*)(secret_sealed), buffer_used, pubkey, address);
  if (ret != 0) {
    LL_CRITICAL("ecdsa_keygen_unseal returns %d", ret);
  }
  std::cout << "PublicKey: " << bufferToHex(pubkey, sizeof pubkey, true) << std::endl;
  std::cout << "Address: " << bufferToHex(address, sizeof address, true) << std::endl;

  sgx_destroy_enclave(eid);
  LL_CRITICAL("Info: all enclave closed successfully.");
}
