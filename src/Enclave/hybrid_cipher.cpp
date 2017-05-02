#include "hybrid_cipher.h"

using namespace std;

const AESIv HybridEncryption::iv = {0x99};


void HybridEncryption::dump_pubkey(const mbedtls_ecp_group *grp, const mbedtls_ecp_point *p, ECPointBuffer buf) {
  size_t olen;
  int ret = mbedtls_ecp_point_write_binary(grp, p, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, buf, PUBLIC_KEY_SIZE);
  if (ret != 0 || olen != PUBLIC_KEY_SIZE) {
    throw runtime_error("mbedtls_ecp_point_write_binary failed");
  }
}

void HybridEncryption::load_pubkey(const mbedtls_ecp_group *grp, mbedtls_ecp_point *p, const ECPointBuffer buf) {
  int ret = mbedtls_ecp_point_read_binary(grp, p, buf, PUBLIC_KEY_SIZE);
  if (ret != 0) {
    throw runtime_error("mbedtls_ecp_point_read_binary failed");
  }
}

const char *HybridEncryption::err(int err) {
  mbedtls_strerror(err, err_msg, sizeof err_msg);
  return err_msg;
}

HybridEncryption::HybridEncryption() {
  ret = 0;

  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init(&entropy);
  ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const unsigned char *) "RANDOM_GEN", 10);

  if (ret != 0) {
    mbedtls_printf("failed in mbedtls_ctr_drbg_seed: %d\n", ret);
    mbedtls_strerror(ret, err_msg, sizeof err_msg);
    throw runtime_error(err_msg);
  }

  mbedtls_ctr_drbg_set_prediction_resistance(&ctr_drbg,
                                             MBEDTLS_CTR_DRBG_PR_OFF);

  // debugging setup
  mbedtls_ssl_init(&dummy_ssl_ctx);
  mbedtls_ssl_config_init(&dummy_ssl_cfg);
  mbedtls_ssl_conf_dbg(&dummy_ssl_cfg, my_debug, NULL);
  if ((ret = mbedtls_ssl_setup(&dummy_ssl_ctx, &dummy_ssl_cfg)) != 0) {
    LL_CRITICAL("failed to setup ssl: %d", ret);
  };

  mbedtls_debug_set_threshold(-1);
}

void HybridEncryption::dump_ciphertext(const HybridCiphertext &ciphertext) {
  hexdump("user pubkey", ciphertext.user_pubkey, ciphertext.USER_PUBKEY_LEN);
  hexdump("aes iv", ciphertext.aes_iv, ciphertext.AES_IV_LEN);
  hexdump("gcm tag", ciphertext.gcm_tag, ciphertext.GCM_TAG_LEN);
  hexdump("cipher text", &ciphertext.data[0], ciphertext.data.size());
}

string HybridEncryption::encode(const HybridCiphertext &ciphertext) {
  vector<uint8_t> tmp_buf;
  tmp_buf.insert(tmp_buf.end(), ciphertext.user_pubkey, ciphertext.user_pubkey + ciphertext.USER_PUBKEY_LEN);
  tmp_buf.insert(tmp_buf.end(), ciphertext.aes_iv, ciphertext.aes_iv + ciphertext.AES_IV_LEN);
  tmp_buf.insert(tmp_buf.end(), ciphertext.gcm_tag, ciphertext.gcm_tag + ciphertext.GCM_TAG_LEN);
  tmp_buf.insert(tmp_buf.end(), ciphertext.data.begin(), ciphertext.data.end());

  char b64[tmp_buf.size() * 2];
  ext::b64_ntop(&tmp_buf[0], tmp_buf.size(), b64, sizeof b64);

  return string(b64);
}

HybridCiphertext HybridEncryption::decode(const string &cipher_b64) {
  HybridCiphertext ciphertext;
  int o_len;

  unsigned char pcipher[cipher_b64.size()];
  unsigned char *cipher = pcipher;
  o_len = ext::b64_pton(cipher_b64.c_str(), pcipher, sizeof pcipher);
  if (o_len == -1) {
    throw runtime_error("failed to base64 encode");
  }

  memcpy(ciphertext.user_pubkey, cipher, ciphertext.USER_PUBKEY_LEN);
  cipher += ciphertext.USER_PUBKEY_LEN;

  memcpy(ciphertext.aes_iv, cipher, ciphertext.AES_IV_LEN);
  cipher += ciphertext.AES_IV_LEN;

  memcpy(ciphertext.gcm_tag, cipher, ciphertext.GCM_TAG_LEN);
  cipher += ciphertext.GCM_TAG_LEN;

  ciphertext.data.insert(ciphertext.data.end(), cipher, cipher + o_len - ciphertext.HEADER_LEN);

  return ciphertext;
}

void HybridEncryption::fill_random(unsigned char *out, size_t len) {
  mbedtls_ctr_drbg_random(&ctr_drbg, out, len);
}

void HybridEncryption::initServer(mbedtls_mpi *seckey, ECPointBuffer pubkey) {
  mbedtls_ecdh_context ecdh_ctx_tc;
  mbedtls_ecdh_init(&ecdh_ctx_tc);

  // load the group
  ret = mbedtls_ecp_group_load(&ecdh_ctx_tc.grp, EC_GROUP);
  CHECK_RET(ret);

  // generate an ephemeral key
  ret = mbedtls_ecdh_gen_public(&ecdh_ctx_tc.grp, &ecdh_ctx_tc.d, &ecdh_ctx_tc.Q,
                                mbedtls_ctr_drbg_random, &ctr_drbg);
  CHECK_RET(ret);

  // release the public key
  dump_pubkey(&ecdh_ctx_tc.grp, &ecdh_ctx_tc.Q, pubkey);

  ret = mbedtls_mpi_copy(seckey, &ecdh_ctx_tc.d);
  CHECK_RET(ret);
}

void HybridEncryption::hybridDecrypt(const HybridCiphertext &ciphertext,
                                     const mbedtls_mpi *secret_key,
                                     vector<uint8_t> &cleartext) {
  mbedtls_ecdh_context ctx_tc;
  mbedtls_ecdh_init(&ctx_tc);

  // load the group
  ret = mbedtls_ecp_group_load(&ctx_tc.grp, EC_GROUP);
  if (ret != 0) {
    mbedtls_printf(" failed\n  ! mbedtls_ecp_group_load returned %d\n", ret);
    throw runtime_error(err(ret));
  }

  // load the sec key
  mbedtls_mpi_copy(&ctx_tc.d, secret_key);

  // load user's public key
  load_pubkey(&ctx_tc.grp, &ctx_tc.Qp, ciphertext.user_pubkey);

  // compute the shared secret
  ret = mbedtls_ecdh_compute_shared(&ctx_tc.grp, &ctx_tc.z,
                                    &ctx_tc.Qp, &ctx_tc.d,
                                    mbedtls_ctr_drbg_random, &ctr_drbg);
  if (ret != 0) {
    mbedtls_printf(" failed\n  ! mbedtls_ecdh_compute_shared returned %d\n", ret);
    throw runtime_error(err(ret));
  }

  mbedtls_debug_print_mpi(&dummy_ssl_ctx, 0, __FILE__, __LINE__, "derived secret", &ctx_tc.z);

  AESKey aes_key;
  mbedtls_mpi_write_binary(&ctx_tc.z, aes_key, sizeof(AESKey));

  cleartext.clear();
  cleartext.resize(ciphertext.data.size());
  aes_gcm_256_dec(aes_key, ciphertext.aes_iv,
                  &ciphertext.data[0], ciphertext.data.size(),
                  ciphertext.gcm_tag, &cleartext[0]);
}

string HybridEncryption::hybridEncrypt(const ECPointBuffer tc_pubkey, const uint8_t *data, size_t data_len) {
  HybridCiphertext ciphertext;
  this->hybridEncrypt(tc_pubkey, iv, data, data_len, ciphertext);

  return this->encode(ciphertext);
}

void HybridEncryption::hybridEncrypt(const ECPointBuffer tc_pubkey,
                                     const AESIv aes_iv,
                                     const uint8_t *data,
                                     size_t data_len,
                                     HybridCiphertext &ciphertext) {
  mbedtls_ecdh_context ctx_user;
  mbedtls_ecdh_init(&ctx_user);

  // load the group
  ret = mbedtls_ecp_group_load(&ctx_user.grp, EC_GROUP);
  CHECK_RET_GO(ret, cleanup);

  // generate an ephemeral key
  ret = mbedtls_ecdh_gen_public(&ctx_user.grp, &ctx_user.d, &ctx_user.Q,
                                mbedtls_ctr_drbg_random, &ctr_drbg);
  CHECK_RET_GO(ret, cleanup);

  dump_pubkey(&ctx_user.grp, &ctx_user.Q, ciphertext.user_pubkey);

  // populate with the tc public key
  ret = mbedtls_mpi_lset(&ctx_user.Qp.Z, 1);
  CHECK_RET_GO(ret, cleanup);
  load_pubkey(&ctx_user.grp, &ctx_user.Qp, tc_pubkey);

  // derive shared secret
  ret = mbedtls_ecdh_compute_shared(&ctx_user.grp, &ctx_user.z,
                                    &ctx_user.Qp, &ctx_user.d,
                                    NULL, NULL);
  CHECK_RET_GO(ret, cleanup);

  mbedtls_debug_print_mpi(&dummy_ssl_ctx, 0, __FILE__, __LINE__, "derived secret", &ctx_user.z);

  // load aes key
  AESKey aes_key;
  mbedtls_mpi_write_binary(&ctx_user.z, aes_key, sizeof(AESKey));

  DEBUG_BUFFER("clear text", data, data_len);
  DEBUG_BUFFER("aes key", aes_key, sizeof(AESKey));

  ciphertext.data.clear();
  ciphertext.data.reserve(data_len);
  aes_gcm_256_enc(aes_key, aes_iv, data, data_len, ciphertext.gcm_tag, ciphertext.data);

  memcpy(ciphertext.aes_iv, aes_iv, sizeof(AESIv));

  DEBUG_BUFFER("cipher", &ciphertext.data[0], ciphertext.data.size());

  cleanup:
  mbedtls_ecdh_free(&ctx_user);
  if (ret) throw runtime_error(err(ret));
}

void HybridEncryption::aes_gcm_256_enc(const AESKey aesKey,
                                       const AESIv iv,
                                       const uint8_t *data,
                                       size_t data_len,
                                       GCMTag tag,
                                       vector<uint8_t> &cipher) {
  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);

  mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, aesKey, 8 * sizeof(AESKey));

  uint8_t _cipher[data_len];
  ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, data_len,
                                  iv, sizeof(AESIv),
                                  NULL, 0,
                                  data,
                                  _cipher, sizeof(GCMTag), tag);

  cipher.insert(cipher.begin(), _cipher, _cipher + data_len);
  mbedtls_gcm_free(&ctx);
  CHECK_RET(ret);
}

void HybridEncryption::aes_gcm_256_dec(const AESKey aesKey,
                                       const AESIv iv,
                                       const uint8_t *ciphertext,
                                       size_t ciphertext_len,
                                       const GCMTag tag,
                                       uint8_t *cleartext) {
  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);

  mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, aesKey, 8 * sizeof(AESKey));

  ret = mbedtls_gcm_auth_decrypt(&ctx, ciphertext_len,
                                 iv, sizeof(AESIv),
                                 NULL, 0,
                                 tag, sizeof(GCMTag),
                                 ciphertext, cleartext);
  mbedtls_gcm_free(&ctx);
  CHECK_RET(ret);
}
