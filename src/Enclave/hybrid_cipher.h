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
#include "Log.h"
#include "Debug.h"
#include "macros.h"

using namespace std;

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

  void dump_pubkey(const mbedtls_ecp_group *grp, const mbedtls_ecp_point *p, ECPointBuffer buf);
  void load_pubkey(const mbedtls_ecp_group *grp, mbedtls_ecp_point *p, const ECPointBuffer buf);
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

  void initServer(mbedtls_mpi *seckey, ECPointBuffer pubkey);
  void hybridDecrypt(const HybridCiphertext &ciphertext, const mbedtls_mpi *secret_key, vector <uint8_t> &cleartext);
 public:
  // public utility functions
  static string encode(const HybridCiphertext &ciphertext);
  static HybridCiphertext decode(const string &cipher_b64);
  static int secretToPubkey(const mbedtls_mpi *seckey, ECPointBuffer pubkey);
  static void print_ciphertxt(const HybridCiphertext &ciphertext);

  HybridEncryption();

  void initServer(ECPointBuffer pubkey);
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
const string& decrypt_query(const uint8_t* data, size_t data_len);

#endif //MBEDTLS_SGX_ENC_H
