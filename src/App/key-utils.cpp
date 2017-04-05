//
// Created by fanz on 4/4/17.
//

#include "key-utils.h"

#include <sgx.h>
#include <sgx_error.h>
#include <sgx_eid.h>

#include <iostream>
#include <string>

#include "Log.h"
#include "macros.h"
#include "external/base64.hxx"
#include "utils.h"

#include "Enclave_u.h"
#include "Converter.h"

using std::string;
using std::cout;
using std::endl;

string unseal_key(sgx_enclave_id_t eid, string sealed_key) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];

  size_t buffer_used = (size_t) ext::b64_pton(sealed_key.c_str(), secret_sealed, sizeof secret_sealed);

  int ret = 0;
  sgx_status_t ecall_ret;
  ecall_ret = ecdsa_keygen_unseal(eid, &ret, (sgx_sealed_data_t *) (secret_sealed), buffer_used, pubkey, address);
  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    LL_CRITICAL("ecall failed");
    print_error_message(ecall_ret);
    LL_CRITICAL("ecdsa_keygen_unseal returns %d", ret);
    throw std::runtime_error("ecdsa_keygen_unseal failed");
  }
  return bufferToHex(address, sizeof address, true);
}
