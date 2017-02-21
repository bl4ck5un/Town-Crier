#include "attestation.h"
#include "utils.h"
#include "Converter.h"

#include <sgx_eid.h>
#include <iostream>

int main() {
  sgx_enclave_id_t eid;
  int ret;
  ret = initialize_tc_enclave(&eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_NOTICE("enclave %lu created", eid);
  }
  std::vector<unsigned char> quote = get_attestation(eid);

  std::cout << bufferToHex(quote, false) << std::endl;
}