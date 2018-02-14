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

#include "App/key_utils.h"

#include <sgx.h>
#include <sgx_eid.h>
#include <sgx_error.h>

#include <iostream>
#include <string>

#include "App/tc_exception.h"
#include "App/converter.h"
#include "App/Enclave_u.h"
#include "App/utils.h"
#include "Common/macros.h"
#include "App/logging.h"
#include "Common/external/base64.hxx"

using std::string;
using std::cout;
using std::endl;

namespace tc {
namespace keyUtils {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("key_utils.cpp"));
}
}

using tc::keyUtils::logger;

/*!
 * unseal the secret signing and return the corresponding address
 * @param[in] eid
 * @param[in] sealed_key
 * @return a string of corresponding address
 */
string unseal_key(sgx_enclave_id_t eid, string sealed_key, tc::keyUtils::KeyType key_type) {
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];

  size_t buffer_used = (size_t)ext::b64_pton(sealed_key.c_str(), secret_sealed,
                                             sizeof secret_sealed);

  int ret = 0;
  sgx_status_t ecall_ret;
  ecall_ret = ecdsa_keygen_unseal(
      eid, &ret, reinterpret_cast<sgx_sealed_data_t*>(secret_sealed),
      buffer_used, pubkey, address);
  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    throw tc::EcallException(
        ecall_ret, "ecdsa_keygen_unseal failed with " + std::to_string(ret));
  }
  switch (key_type) {
    case tc::keyUtils::ECDSA_KEY:
      return bufferToHex(address, sizeof address, true);
    case tc::keyUtils::HYBRID_ENCRYPTION_KEY:
      // take special care of the first byte
      unsigned char _pubkey65b[65];
      _pubkey65b[0] = 0x04;
      memcpy(_pubkey65b + 1, pubkey, 64);
      char _base64_pubkey[2 * sizeof pubkey];
      ret = ext::b64_ntop(_pubkey65b, 65, _base64_pubkey, sizeof _base64_pubkey);
      if (ret == -1)
        throw std::runtime_error("unknown error");
      return string(_base64_pubkey);
    default:
      throw std::runtime_error("unknown key type");
  }
}

void provision_key(sgx_enclave_id_t eid, string sealed_key, tc::keyUtils::KeyType type) {
  unsigned char _sealed_key_buf[SECRETKEY_SEALED_LEN];
  auto buffer_used = (size_t)ext::b64_pton(sealed_key.c_str(), _sealed_key_buf, sizeof _sealed_key_buf);

  int ret = 0;
  sgx_status_t ecall_ret;

  switch (type) {
    case tc::keyUtils::ECDSA_KEY:
      ecall_ret = tc_provision_ecdsa_key(eid, &ret,
                                          reinterpret_cast<sgx_sealed_data_t*>(_sealed_key_buf), buffer_used);
      break;
    case tc::keyUtils::HYBRID_ENCRYPTION_KEY:
      ecall_ret = tc_provision_hybrid_key(eid, &ret,
                                          reinterpret_cast<sgx_sealed_data_t*>(_sealed_key_buf), buffer_used);
      break;
    default:
      LL_CRITICAL("unknown key type");
      ecall_ret = SGX_ERROR_UNEXPECTED;
      ret = -1;
  }

  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    throw tc::EcallException(ecall_ret, "tc_provision_key returns " + std::to_string(ret));
  }
}
