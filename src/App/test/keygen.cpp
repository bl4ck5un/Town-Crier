//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include <iostream>
#include "gtest/gtest.h"

#include "App/converter.h"
#include "App/Enclave_u.h"
#include "App/test/SGXTestBase.h"
#include "App/utils.h"
#include "App/debug.h"
#include "Common/macros.h"


class Keygen : public SGXTestBase {};

TEST_F(Keygen, keygen) {
  sgx_status_t st;
  int ret;
  st = keygen_test(eid, &ret);
  ASSERT_EQ(0, st);
  ASSERT_EQ(0, ret);
}

TEST_F(Keygen, signingKey) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey_ref[PUBKEY_LEN];
  unsigned char address_ref[ADDRESS_LEN];

  // call into enclave to fill the above three buffers
  size_t buffer_used = 0;
  int ret;
  sgx_status_t ecall_status;
  ecall_status = ecdsa_keygen_seal(eid, &ret, secret_sealed, &buffer_used,
                                   pubkey_ref, address_ref);
  if (ecall_status != SGX_SUCCESS || ret != 0) {
    std::cerr << "ecall failed" << std::endl;
    print_error_message(ecall_status);
    std::cerr << "ecdsa_keygen_seal returns " << ret << std::endl;
    FAIL();
  }

  unsigned char pubkey_result[PUBKEY_LEN];
  unsigned char address_result[ADDRESS_LEN];
  tc_get_address(eid, &ret, pubkey_result, address_result);
  ASSERT_EQ(ret, TC_KEY_NOT_PROVISIONED);

  ecall_status = tc_provision_ecdsa_key(
      eid, &ret, reinterpret_cast<sgx_sealed_data_t*>(secret_sealed),
      buffer_used);
  if (SGX_SUCCESS != ecall_status || ret != 0) {
    std::fprintf(stderr, "ecall failed");
    print_error_message(ecall_status);
    std::fprintf(stderr, "ecdsa_keygen_seal returns %d", ret);
    FAIL();
  }

  tc_get_address(eid, &ret, pubkey_result, address_result);

  ASSERT_EQ(0, memcmp(pubkey_ref, pubkey_result, PUBKEY_LEN));
  ASSERT_EQ(0, memcmp(address_ref, address_result, ADDRESS_LEN));
}

TEST_F(Keygen, HybridKey) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  // Hybrid pubkey length = 65, an EC point
  unsigned char pubkey_ref[65];
  unsigned char address_ref[ADDRESS_LEN];

  // call into enclave to fill the above three buffers
  size_t buffer_used = 0;
  int ret;
  sgx_status_t ecall_status;
  ecall_status = ecdsa_keygen_seal(eid, &ret, secret_sealed, &buffer_used,
                                   pubkey_ref + 1, address_ref);
  if (ecall_status != SGX_SUCCESS || ret != 0) {
    std::fprintf(stderr, "ecall failed");
    print_error_message(ecall_status);
    std::fprintf(stderr, "ecdsa_keygen_seal returns %d", ret);
    FAIL();
  }

  // the first byte of EC point binary is always 4
  // this byte is not returned by ecdsa_keygen_seal following Ethereum
  // conventions.
  // but we need it for hybrid encryption.
  pubkey_ref[0] = 0x04;

  unsigned char pubkey_result[65];
  ecall_status = tc_provision_hybrid_key(eid,
                &ret,
                reinterpret_cast<sgx_sealed_data_t*>(secret_sealed),
                buffer_used);
  if (SGX_SUCCESS != ecall_status || ret != 0) {
    std::fprintf(stderr, "ecall failed");
    print_error_message(ecall_status);
    std::fprintf(stderr, "ecdsa_keygen_seal returns %d", ret);
    FAIL();
  }

  tc_get_hybrid_pubkey(eid, &ret, pubkey_result);

  ASSERT_EQ(0, memcmp(pubkey_ref, pubkey_result, 65));
}
