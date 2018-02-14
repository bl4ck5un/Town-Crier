/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
 * the Center for Technology Licensing at Cornell University as D-7364, developed
 * through research conducted at Cornell University, and its associated copyrights
 * solely for educational, research and non-profit purposes without fee is hereby
 * granted, provided that the user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling the
 * TownCrier source code. No other rights to use TownCrier and its associated
 * copyrights for any other purpose are granted herein, whether commercial or
 * non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial products or use
 * TownCrier and its associated copyrights for commercial purposes must contact the
 * Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
 * Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
 * FAX: 607-254-5454 for a commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
 * CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
 * OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
 * WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
 * PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
 * INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science Foundation
 * (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
 * Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
 * VMWare Research Award.
 */

#ifndef MBEDTLS_SGX_ENC_H
#define MBEDTLS_SGX_ENC_H

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/config.h>
#include <mbedtls/platform.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecp.h>
#include <mbedtls/gcm.h>

#include <string.h>

#include <exception>
#include <vector>
#include <string>

#include "external/base64.hxx"
#include "log.h"
#include "debug.h"
#include "macros.h"
#include "Enclave_t.h"
#include "../Common/macros.h"

using namespace std;

#define PREDEFINED_HYBRID_SECKEY "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"
#undef PREDEFINED_HYBRID_SECKEY

#define CHECK_RET(ret) do { if (ret != 0) { throw runtime_error(err(ret)); }} while (0);
#define CHECK_RET_GO(ret, label) do { if (ret != 0) { goto label; }} while (0);

#define DEBUG_BUFFER(title, buf, len) do { \
  mbedtls_debug_print_buf(&dummy_ssl_ctx, 0, __FILE__,__LINE__, title, buf, len); } \
 while (0);

static void my_debug(void *ctx, int level, const char *file, int line,
                     const char *str) {
  (void) ctx;
  (void) level;

  mbedtls_printf("%s:%d: %s", file, line, str);
}

typedef uint8_t AESKey[32];
typedef uint8_t AESIv[32];
typedef uint8_t GCMTag[16];
typedef uint8_t ECPointBuffer[65];

class HybridCiphertext {
 public:
  static const size_t USER_PUBKEY_LEN = sizeof(ECPointBuffer);
  static const size_t AES_IV_LEN = sizeof(AESIv);
  static const size_t GCM_TAG_LEN = sizeof(GCMTag);
  static const size_t HEADER_LEN = USER_PUBKEY_LEN + AES_IV_LEN + GCM_TAG_LEN;

  ECPointBuffer user_pubkey;
  AESIv aes_iv;
  GCMTag gcm_tag;
  vector <uint8_t> data;

  HybridCiphertext() {};
  void toString();
};

class HybridEncryption {
 public:
  static const mbedtls_ecp_group_id EC_GROUP = MBEDTLS_ECP_DP_SECP256K1;
  static const AESIv iv;
  static const size_t PUBLIC_KEY_SIZE = 65;

 private:
  // general setup
  char err_msg[1024];
  int ret;
  uint8_t buf[100];

  // rng setup
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;

  // only used for debugging
  mbedtls_ssl_context dummy_ssl_ctx;
  mbedtls_ssl_config dummy_ssl_cfg;

  void storePubkey(const mbedtls_ecp_group *grp, const mbedtls_ecp_point *p, ECPointBuffer buf);
  void loadPubkey(const mbedtls_ecp_group *grp, mbedtls_ecp_point *p, const uint8_t *buf);
  const char *err(int err);

  void hybridEncrypt(const ECPointBuffer tc_pubkey,
                     const AESIv aes_iv,
                     const uint8_t *data,
                     size_t data_len,
                     HybridCiphertext &ciphertext);

  void aes_gcm_256_enc(const AESKey aesKey, const AESIv iv,
                       const uint8_t *data, size_t data_len,
                       GCMTag tag, vector <uint8_t> &cipher);

  void aes_gcm_256_dec(const AESKey aesKey, const AESIv iv,
                       const uint8_t *ciphertext, size_t ciphertext_len,
                       const GCMTag tag, uint8_t *cleartext);

  void hybridDecrypt(const HybridCiphertext &ciphertext, const mbedtls_mpi *secret_key, vector <uint8_t> &cleartext);
 public:
  // public utility functions
  static string encode(const HybridCiphertext &ciphertext);
  static HybridCiphertext decode(const string &cipher_b64);
  static int secretToPubkey(const mbedtls_mpi *seckey, ECPointBuffer pubkey);

  HybridEncryption();

  void queryPubkey(ECPointBuffer pubkey);
  string hybridEncrypt(const ECPointBuffer tc_pubkey, const uint8_t *data, size_t data_len);
  void hybridDecrypt(const HybridCiphertext &ciphertext, vector <uint8_t> &cleartext);
};


// key-util
class DecryptionException: public std::exception {
 private:
  const string reason;
 public:
  DecryptionException(string reason): reason(reason) {}
  const char* what() const throw() { return reason.c_str(); }
};


int tc_provision_hybrid_key(const sgx_sealed_data_t *secret, size_t secret_len);
int tc_get_hybrid_pubkey(ECPointBuffer pubkey);
const string decrypt_query(const uint8_t* data, size_t data_len);

#endif //MBEDTLS_SGX_ENC_H
