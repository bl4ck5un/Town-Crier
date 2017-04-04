//
// Created by fanz on 6/11/16.
//

#include <iostream>
#include "gtest/gtest.h"

#include "../utils.h"
#include "macros.h"
#include "../Converter.h"

using namespace std;

class KeygenTestSuite: public ::testing::Test
{
 protected:
  sgx_enclave_id_t eid;
  KeygenTestSuite()
  {
    ;
  }
  ~KeygenTestSuite()
  {
    ;
  }

  virtual void SetUp()
  {
    initialize_enclave(ENCLAVE_FILENAME, &eid);
  }

  virtual void TearDown()
  {
    sgx_destroy_enclave(eid);
  }
};

TEST_F (KeygenTestSuite, keygen) {
  unsigned char pubkey[64];
  unsigned char address[20];
  sgx_status_t st;
  int ret;
  st = keygen_test(eid, &ret);
  EXPECT_EQ(0, ret);
}

TEST_F(KeygenTestSuite, provision) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey_ref[PUBKEY_LEN];
  unsigned char address_ref[ADDRESS_LEN];

  // call into enclave to fill the above three buffers
  size_t buffer_used = 0;
  int ret;
  sgx_status_t ecall_status;
  ecall_status = ecdsa_keygen_seal(eid, &ret, secret_sealed, &buffer_used, pubkey_ref, address_ref);
  if (ecall_status != SGX_SUCCESS || ret != 0) {
    LL_CRITICAL("ecall failed");
    print_error_message(ecall_status);
    LL_CRITICAL("ecdsa_keygen_seal returns %d", ret);
    FAIL();
  }

  cout << "PublicKey: " << bufferToHex(pubkey_ref, sizeof pubkey_ref, true) << endl;
  cout << "Address: " << bufferToHex(address_ref, sizeof address_ref, true) << endl;

  unsigned char pubkey_result[PUBKEY_LEN];
  unsigned char address_result[ADDRESS_LEN];
  tc_get_address(eid, &ret, pubkey_result, address_result);
  ASSERT_EQ(ret, TC_KEY_NOT_PROVISIONED);

  ecall_status = tc_provision_key(eid, &ret, (sgx_sealed_data_t*) secret_sealed, buffer_used);
  if (SGX_SUCCESS != ecall_status || ret != 0) {
    LL_CRITICAL("ecall failed");
    print_error_message(ecall_status);
    LL_CRITICAL("ecdsa_keygen_seal returns %d", ret);
    FAIL();
  }

  tc_get_address(eid, &ret, pubkey_result, address_result);

  cout << "PublicKey: " << bufferToHex(address_result, sizeof address_result, true) << endl;
  cout << "Address: " << bufferToHex(address_result, sizeof address_result, true) << endl;

  ASSERT_EQ(0, memcmp(pubkey_ref, pubkey_result, PUBKEY_LEN));
  ASSERT_EQ(0, memcmp(address_ref, address_result, ADDRESS_LEN));
}