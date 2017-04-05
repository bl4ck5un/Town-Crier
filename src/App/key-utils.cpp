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
#include "tc-exception.hxx"

using std::string;
using std::cout;
using std::endl;

/*!
 * unseal the secret signing and return the corresponding address
 * @param[in] eid
 * @param[in] sealed_key
 * @return a string of corresponding address
 */
string unseal_key(sgx_enclave_id_t eid, string sealed_key) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];

  size_t buffer_used = (size_t) ext::b64_pton(sealed_key.c_str(), secret_sealed, sizeof secret_sealed);

  int ret = 0;
  sgx_status_t ecall_ret;
  ecall_ret = ecdsa_keygen_unseal(eid, &ret, (sgx_sealed_data_t *) (secret_sealed), buffer_used, pubkey, address);
  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    throw tc::EcallException(ecall_ret, "ecdsa_keygen_unseal failed with " + std::to_string(ret));
  }
  return bufferToHex(address, sizeof address, true);
}

void provision_key(sgx_enclave_id_t eid, string sealed_key) {
  unsigned char _sealed_key_buf[SECRETKEY_SEALED_LEN];

  size_t buffer_used = (size_t) ext::b64_pton(sealed_key.c_str(), _sealed_key_buf, sizeof _sealed_key_buf);

  int ret = 0;
  sgx_status_t ecall_ret;
  ecall_ret = tc_provision_key(eid, &ret, (sgx_sealed_data_t*) _sealed_key_buf, buffer_used);
  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    throw tc::EcallException(ecall_ret, "tc_provision_key returns " + std::to_string(ret));
  }
}
