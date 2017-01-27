#include "eth_ecdsa.h"
#include "external/keccak.h"
#include "Debug.h"
#include "Log.h"
#include "glue.h"
#include "mbedtls/error.h"
#include "macros.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif

#if defined(MBEDTLS_ECDSA_C) && \
    defined(MBEDTLS_ENTROPY_C) && defined(MBEDTLS_CTR_DRBG_C)
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/bignum.h"

#include <string.h>
#include <mbedtls/ecp.h>

#endif

#include "mbedtls/sha256.h"

#define SIGN_DEBUG
#undef  SIGN_DEBUG

/*
 * Uncomment to show key and signature details
 */
#define VERBOSE
#undef  VERBOSE

#include "Debug.h"

/*
 * Uncomment to force use of a specific curve
 */
#define ECPARAMS    MBEDTLS_ECP_DP_SECP256K1

// pubkey: 64 Bytes
// SHA3-256: 32 Bytes
// use lower 160 bits as address
/*
---- ADDRESS -------------------------------
SEC: cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8
PUB: bb48ae3726c5737344a54b3463fec499cb108a7d11ba137ba3c7d043bd6d7e14994f60462a3f91550749bb2ae5411f22b7f9bee79956a463c308ad508f3557df
ADR: 89b44e4d3c81ede05d0f5de8d1a68f754d73d997
*/

#define FROM_PRIVATE_KEY "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"

int ecdsa_keygen(unsigned char *pubkey, unsigned char *address) {
  return __ecdsa_keygen(NULL, pubkey, address);
}

/*!
 * @brief generate a key pair (or generate a pubkey and address from a secret key)
 *        see cpp-ethereum/utils/secp256k1/secp256k1.c:249 secp256k1_ec_pubkey_create
 * @param seckey NULL or a pointer to a secret key
 * @param pubkey [out,size=64] output buffer for the public key
 * @param address [out,size=20] output buffer for the address
 * @return 0 if succeed
 */
int __ecdsa_keygen(const mbedtls_mpi *seckey, unsigned char *pubkey, unsigned char *address) {
  mbedtls_ecdsa_context ctx;
  unsigned char __pubkey[65];
  unsigned char __address[32];
  size_t buflen = 0;
  int ret;

  mbedtls_ecdsa_init(&ctx);
  mbedtls_ecp_group_load(&ctx.grp, ECPARAMS);

  if (seckey != NULL) {
    mbedtls_mpi_copy(&ctx.d, seckey);
  } else {
    mbedtls_mpi_fill_random(&ctx.d, ctx.grp.nbits / 8, mbedtls_sgx_drbg_random, NULL);
  }

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
    LL_CRITICAL("ecp serialization is incorrect olen=%d", buflen);
  }

  ret = keccak(__pubkey + 1, 64, __address, 32);
  if (ret != 0) {
    LL_CRITICAL("keccak returned %d", ret);
    return -1;
  }

  // copy to user space
  memcpy(pubkey, __pubkey + 1, 64);
  memcpy(address, __address + 12, 20);

  return 0;
}

//!
//! \brief unseal the secret and return the address
//! \param secret
//! \param secret_len
//! \param address
//! \return
#include <sgx_tseal.h>
int unseal_to_address(sgx_sealed_data_t *secret, size_t secret_len, unsigned char *address) {
  uint32_t MACtext_length = sgx_get_add_mac_txt_len(secret);
  uint32_t decrypted_text_length = sgx_get_encrypt_txt_len(secret);
  uint8_t y[decrypted_text_length];
  if (sgx_unseal_data(secret, 0, &MACtext_length, y, &decrypted_text_length) != SGX_SUCCESS) return -1;

  mbedtls_mpi seckey;
  mbedtls_mpi_init(&seckey);
  mbedtls_mpi_read_binary(&seckey, y, sizeof y);

  unsigned char pubkey[PUBKEY_LEN];
  return __ecdsa_keygen(&seckey, pubkey, address);
}

int ecdsa_keygen_seal(unsigned char *sealed, size_t *olen, unsigned char *pubkey, unsigned char *address) {
  mbedtls_mpi secret;
  mbedtls_mpi_init(&secret);

  mbedtls_ecp_group grp;
  mbedtls_ecp_group_init(&grp);
  mbedtls_ecp_group_load(&grp, ECPARAMS);
  mbedtls_mpi_fill_random(&secret, grp.nbits / 8, mbedtls_sgx_drbg_random, NULL);
  LL_LOG("secret length is %d", grp.nbits / 8);

  int ret = 0;
  unsigned char secret_buffer[32];
  if (mbedtls_mpi_write_binary(&secret, secret_buffer, sizeof secret_buffer) != 0) {
    LL_CRITICAL("can't run secret to buffer");
    return -1;
  }

  uint32_t len = sgx_calc_sealed_data_size(0, sizeof(secret_buffer));
  sgx_sealed_data_t *p = (sgx_sealed_data_t *) sealed;
  if (sgx_seal_data(0, 0, sizeof secret_buffer, secret_buffer, len, p) != SGX_SUCCESS)
    return -1;

  *olen = len;

  if (__ecdsa_keygen(&secret, pubkey, address) != 0) {
    LL_CRITICAL("failed to keygen");
    return -1;
  }
  return 0;
}

DEPRECATED(void keygen(mbedtls_ecdsa_context *ctx)) {
  int ret;
  mbedtls_ecp_group_load(&ctx->grp, ECPARAMS);
  ret = mbedtls_mpi_read_string(&ctx->d, 16, FROM_PRIVATE_KEY);
  if (ret != 0) {
    mbedtls_printf("Error: mbedtls_mpi_read_string returned %d\n", ret);
    return;
  }
  ret = mbedtls_ecp_mul(&ctx->grp, &ctx->Q, &ctx->d, &ctx->grp.G, NULL, NULL);
  if (ret != 0) {
    mbedtls_printf("Error: mbedtls_ecp_mul returned %d\n", ret);
  }
}

int sign(uint8_t *data, int in_len, uint8_t *rr, uint8_t *ss, uint8_t *vv) {
  int ret;
  mbedtls_ecdsa_context ctx_sign, ctx_verify;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;

  mbedtls_mpi r, s;

  // here begins statements
  mbedtls_mpi_init(&r);
  mbedtls_mpi_init(&s);
  mbedtls_ecdsa_init(&ctx_sign);
  mbedtls_ecdsa_init(&ctx_verify);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  ret = 1;

#ifdef SIGN_DEBUG
  dump_buf("data: ", data, in_len);
#endif
  keygen(&ctx_sign);
#ifdef SIGN_DEBUG
  dump_pubkey( "pk: ", &ctx_sign );
#endif

  // sign
  /*
  ret = mbedtls_ecdsa_sign_bitcoin(&ctx_sign.grp, &r, &s, &v, &ctx_sign.d,
      data, in_len, MBEDTLS_MD_SHA256);
  */
  ret = mbedtls_ecdsa_sign_with_v(&ctx_sign.grp, &r, &s, vv, &ctx_sign.d,
                                  data, in_len, mbedtls_sgx_drbg_random, NULL);
  if (ret != 0) {
    LL_CRITICAL("mbedtls_ecdsa_sign_bitcoin returned %#x", ret);
    goto exit;
  }


// #define SIGN_DEBUG
#ifdef SIGN_DEBUG
  dump_mpi("r: ", &r);
  dump_mpi("s: ", &s);
#endif // SIGN_DEBUG

  mbedtls_mpi_write_binary(&r, rr, 32);
  mbedtls_mpi_write_binary(&s, ss, 32);

#ifdef SIGN_DEBUG
  dump_buf("r_buf: ", rr, 32);
  dump_buf("s_buf: ", ss, 32);
  dump_buf("v_buf: ", vv, 1);
#endif // SIGN_DEBUG

  ret = mbedtls_ecdsa_verify(&ctx_sign.grp, data, in_len, &ctx_sign.Q, &r, &s);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_ecdsa_verify returned %#x", ret);
    goto exit;
  } else {
#ifdef SIGN_DEBUG
    mbedtls_printf("Verified!\n");
#else
    ;
#endif
  }

  exit:
  if (ret != 0) {
    char error_buf[100];
    mbedtls_strerror(ret, error_buf, 100);
    LL_CRITICAL("Last error was: -0x%X - %s", -ret, error_buf);
  }
  mbedtls_ecdsa_free(&ctx_verify);
  mbedtls_ecdsa_free(&ctx_sign);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
  mbedtls_mpi_free(&r);
  mbedtls_mpi_free(&s);
  return (ret);
}
