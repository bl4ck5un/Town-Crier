#include <mbedtls-SGX/include/mbedtls/bignum.h>
#include "hybrid_cipher.h"

using namespace std;

#define PREDEFINED_HYBRID_SECKEY "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"

static mbedtls_mpi g_secret_hybrid_key;

int tc_provision_hybrid_key(const sgx_sealed_data_t *secret, size_t secret_len) {
  // used by edge8r
  (void) secret_len;

  uint32_t decrypted_text_length = sgx_get_encrypt_txt_len(secret);
  uint8_t y[decrypted_text_length];
  sgx_status_t st;

  st = sgx_unseal_data(secret, NULL, 0, y, &decrypted_text_length);
  if (st != SGX_SUCCESS) {
    LL_CRITICAL("unseal returned %#x", st);
    return -1;
  }

  // initialize the global secret key
  mbedtls_mpi_init(&g_secret_hybrid_key);
  return mbedtls_mpi_read_binary(&g_secret_hybrid_key, y, sizeof y);
}

int tc_get_hybrid_pubkey(ECPointBuffer pubkey) {
  if (g_secret_hybrid_key.p == NULL) {
    LL_CRITICAL("key has not been provisioned yet. Call tc_provision_key() first");
    return TC_KEY_NOT_PROVISIONED;
  }
  return HybridEncryption::secretToPubkey(&g_secret_hybrid_key, pubkey);
}


const string decrypt_query(const uint8_t* data, size_t data_len) {
  HybridEncryption dec_ctx;
  ECPointBuffer tc_pubkey;
  dec_ctx.initServer(tc_pubkey);

  string cipher_b64(data, data + data_len);
  hexdump("encrypted query: ", data, data_len);

  try {
    HybridCiphertext cipher = dec_ctx.decode(cipher_b64);
    vector<uint8_t> cleartext;
    dec_ctx.hybridDecrypt(cipher, cleartext);
    hexdump("decrypted message", &cleartext[0], cleartext.size());

    // decrypted message is the base64 encoded data
    string encoded_message(cleartext.begin(), cleartext.end());
    return encoded_message;
  }
  catch (const std::exception &e) {
    LL_CRITICAL("decryption error: %s. See dump above.", e.what());
    throw DecryptionException(e.what());
  }
  catch (...) {
    LL_CRITICAL("unknown exception happened while decrypting. See dump above.");
    throw DecryptionException("unknown exception");
  }
}

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

void HybridEncryption::print_ciphertxt(const HybridCiphertext &ciphertext) {
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

void HybridEncryption::initServer(ECPointBuffer pubkey) {
#ifndef PREDEFINED_HYBRID_SECKEY
  if (g_secret_hybrid_key.p == NULL) {
    LL_CRITICAL("key not provisioned yet");
  }
#else
  LL_CRITICAL("*** PREDEFINED SECRET KEY IS USED ***");
  LL_CRITICAL("*** DISABLE THIS BEFORE DEPLOY ***");
  ret = mbedtls_mpi_read_string(&g_secret_hybrid_key, 16, PREDEFINED_HYBRID_SECKEY);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_mpi_read_string returned %d", ret);
    return;
  }
  LL_DEBUG("before secretToPubkey");
  HybridEncryption::secretToPubkey(&g_secret_hybrid_key, pubkey);
  LL_DEBUG("after secretToPubkey");
#endif
}

void HybridEncryption::hybridDecrypt(const HybridCiphertext &ciphertext, vector<uint8_t> &cleartext) {
  if (g_secret_hybrid_key.p == NULL) {
    throw runtime_error("hybrid key not provisioned yet. Run initServer() first");
  }
  this->hybridDecrypt(ciphertext, &g_secret_hybrid_key, cleartext);
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

int HybridEncryption::secretToPubkey(const mbedtls_mpi *seckey, ECPointBuffer pubkey) {
  if (pubkey == NULL || seckey == NULL) {
    return -1;
  }

  mbedtls_ecdsa_context ctx;
  unsigned char __pubkey[65];
  size_t buflen = 0;
  int ret;

  mbedtls_ecdsa_init(&ctx);
  mbedtls_ecp_group_load(&ctx.grp, EC_GROUP);

  mbedtls_mpi_copy(&ctx.d, seckey);

  ret = mbedtls_ecp_mul(&ctx.grp, &ctx.Q, &ctx.d, &ctx.grp.G, NULL, NULL);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_ecp_mul returned %d", ret);
    return -1;
  }

  ret = mbedtls_ecp_point_write_binary(&ctx.grp, &ctx.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &buflen, __pubkey, 65);
  if (ret == MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL) {
    LL_CRITICAL("buffer too small");
    return -1;
  } else if (ret == MBEDTLS_ERR_ECP_BAD_INPUT_DATA) {
    LL_CRITICAL("bad input data");
    return -1;
  }
  if (buflen != 65) {
    LL_CRITICAL("ecp serialization is incorrect olen=%ld", buflen);
  }

  // copy to user space
  memcpy(pubkey, __pubkey, 65);
  return 0;
}
