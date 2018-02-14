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

#include <mbedtls-SGX/include/mbedtls/bignum.h>
#include <sgx_tseal.h>
#include <string.h>
#include <stdexcept>
#include <string>

#include "debug.h"
#include "log.h"
#include "eth_ecdsa.h"
#include "external/keccak.h"
#include "../Common/macros.h"

using std::runtime_error;

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif

#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ENTROPY_C) && \
    defined(MBEDTLS_CTR_DRBG_C)
#include "mbedtls/bignum.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#endif
#include "mbedtls/sha256.h"
#define SIGN_DEBUG
#undef SIGN_DEBUG

#define ECPARAMS MBEDTLS_ECP_DP_SECP256K1

// pubkey: 64 Bytes
// SHA3-256: 32 Bytes
// use lower 160 bits as address
/*
---- ADDRESS -------------------------------
SEC: cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8
PUB: bb48ae3726c5737344a54b3463fec499cb108a7d11ba137ba3c7d043bd6d7e14994f60462a3f91550749bb2ae5411f22b7f9bee79956a463c308ad508f3557df
ADR: 89b44e4d3c81ede05d0f5de8d1a68f754d73d997
*/

#define PREDEFINED_SECKEY \
  "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"
#undef PREDEFINED_SECKEY
static mbedtls_mpi g_secret_key;

/*!
 * @brief generate a key pair (or generate a pubkey and address from a secret
 * key)
 *        see cpp-ethereum/utils/secp256k1/secp256k1.c:249
 * secp256k1_ec_pubkey_create
 * @param seckey NULL or a pointer to a secret key
 * @param pubkey [out,size=64] output buffer for the public key
 * @param address [out,size=20] output buffer for the address
 * @return 0 if succeed
 */
int __ecdsa_seckey_to_pubkey(const mbedtls_mpi *seckey, unsigned char *pubkey,
                             unsigned char *address) {
  if (pubkey == NULL || address == NULL || seckey == NULL) {
    return -1;
  }

  mbedtls_ecdsa_context ctx;
  unsigned char __pubkey[65];
  unsigned char __address[32];
  size_t buflen = 0;
  int ret;

  mbedtls_ecdsa_init(&ctx);
  mbedtls_ecp_group_load(&ctx.grp, ECPARAMS);

  mbedtls_mpi_copy(&ctx.d, seckey);

  ret = mbedtls_ecp_mul(&ctx.grp, &ctx.Q, &ctx.d, &ctx.grp.G, NULL, NULL);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_ecp_mul returned %d", ret);
    return -1;
  }

  ret = mbedtls_ecp_point_write_binary(
      &ctx.grp, &ctx.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &buflen, __pubkey, 65);
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

/*!
 * recover (unseal) the public key and address from the sealed secret key.
 * @param secret
 * @param secret_len
 * @param pubkey
 * @param address
 * @return
 */
int ecdsa_keygen_unseal(const sgx_sealed_data_t *secret, size_t secret_len,
                        unsigned char *pubkey, unsigned char *address) {
  // used by edge8r
  (void) secret_len;

  uint32_t decrypted_text_length = sgx_get_encrypt_txt_len(secret);
  uint8_t y[decrypted_text_length];
  sgx_status_t st;

  st = sgx_unseal_data(secret, NULL, 0, y, &decrypted_text_length);
  if (st != SGX_SUCCESS) {
    LL_CRITICAL("unseal returned %x", st);
    return -1;
  }

  // initialize the local secret key
  mbedtls_mpi secret_key;
  mbedtls_mpi_init(&secret_key);
  mbedtls_mpi_read_binary(&secret_key, y, sizeof y);

  return __ecdsa_seckey_to_pubkey(&secret_key, pubkey, address);
}

/*!
 * Set the global secret key
 * @param secret
 * @param secret_len
 * @param pubkey
 * @param address
 * @return
 */
int tc_provision_ecdsa_key(const sgx_sealed_data_t *secret, size_t secret_len) {
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
  mbedtls_mpi_init(&g_secret_key);
  return mbedtls_mpi_read_binary(&g_secret_key, y, sizeof y);
}

/*!
 * get the current address of Town Crier wallet
 * @param pubkey
 * @param address
 * @return
 */
int tc_get_address(unsigned char *pubkey, unsigned char *address) {
  if (g_secret_key.p == NULL) {
    LL_CRITICAL(
        "key has not been provisioned yet. Call tc_provision_key() first");
    return TC_KEY_NOT_PROVISIONED;
  }
  return __ecdsa_seckey_to_pubkey(&g_secret_key, pubkey, address);
}

/*!
 * generate a new key pair and return the sealed secret key, the public key and
 * the address
 * @param o_sealed
 * @param olen
 * @param o_pubkey
 * @param o_address
 * @return
 */
int ecdsa_keygen_seal(unsigned char *o_sealed, size_t *olen,
                      unsigned char *o_pubkey, unsigned char *o_address) {
  mbedtls_ecp_group grp;
  int ret = 0;

  mbedtls_mpi secret;
  mbedtls_mpi_init(&secret);

  mbedtls_ecp_group_init(&grp);
  mbedtls_ecp_group_load(&grp, ECPARAMS);
#ifdef PREDEFINED_SECKEY
  LL_CRITICAL("*** PREDEFINED SECRET KEY IS USED ***");
  LL_CRITICAL("*** DISABLE THIS BEFORE DEPLOY ***");
  ret = mbedtls_mpi_read_string(&secret, 16, PREDEFINED_SECKEY);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_mpi_read_string returned %d", ret);
    return -1;
  }
#else
  mbedtls_mpi_fill_random(&secret, grp.nbits / 8, mbedtls_sgx_drbg_random,
                          NULL);
#endif

  unsigned char secret_buffer[32];
  if (mbedtls_mpi_write_binary(&secret, secret_buffer, sizeof secret_buffer) !=
      0) {
    LL_CRITICAL("can't run secret to buffer");
    ret = -1;
    goto exit;
  }

  // seal the data
  {
    uint32_t len = sgx_calc_sealed_data_size(0, sizeof(secret_buffer));
    sgx_sealed_data_t *seal_buffer = (sgx_sealed_data_t *) malloc(len);
    LL_LOG("sealed secret length is %d", len);

    sgx_status_t st = sgx_seal_data(0, NULL, sizeof secret_buffer,
                                    secret_buffer, len, seal_buffer);
    if (st != SGX_SUCCESS) {
      LL_LOG("Failed to seal. Ecall returned %d", st);
      ret = -1;
      goto exit;
    }

    *olen = len;
    memcpy(o_sealed, seal_buffer, len);
    free(seal_buffer);
  }

  if (__ecdsa_seckey_to_pubkey(&secret, o_pubkey, o_address) != 0) {
    LL_CRITICAL("failed to get public key");
    ret = -1;
    goto exit;
  }
  LL_LOG("returning from keygen_seal");

  exit:
  mbedtls_mpi_free(&secret);
  mbedtls_ecp_group_free(&grp);
  return ret;
}

int ecdsa_sign(const uint8_t *data, size_t in_len, uint8_t *rr, uint8_t *ss,
               uint8_t *vv) {
  int ret;
  mbedtls_ecdsa_context ctx_sign, ctx_verify;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_mpi r, s;

  mbedtls_mpi_init(&r);
  mbedtls_mpi_init(&s);
  mbedtls_ecdsa_init(&ctx_sign);
  mbedtls_ecdsa_init(&ctx_verify);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  mbedtls_ecp_group_load(&ctx_sign.grp, ECPARAMS);

  if (g_secret_key.p == NULL) {
    LL_CRITICAL(
        "signing key not provisioned yet. Call tc_provision_key() first");
    return -1;
  }
  ret = mbedtls_mpi_copy(&ctx_sign.d, &g_secret_key);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_mpi_copy returned %d", ret);
    return -1;
  }
  ret = mbedtls_ecp_mul(&ctx_sign.grp, &ctx_sign.Q, &ctx_sign.d,
                        &ctx_sign.grp.G, NULL, NULL);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_ecp_mul returned %d", ret);
    return -1;
  }

  ret = mbedtls_ecdsa_sign_with_v(&ctx_sign.grp, &r, &s, vv, &ctx_sign.d, data,
                                  in_len, mbedtls_sgx_drbg_random, NULL);
  if (ret != 0) {
    LL_CRITICAL("mbedtls_ecdsa_sign_bitcoin returned %#x", ret);
    goto exit;
  }

  mbedtls_mpi_write_binary(&r, rr, 32);
  mbedtls_mpi_write_binary(&s, ss, 32);

  ret = mbedtls_ecdsa_verify(&ctx_sign.grp, data, in_len, &ctx_sign.Q, &r, &s);
  if (ret != 0) {
    LL_CRITICAL("Error: mbedtls_ecdsa_verify returned %#x", ret);
    goto exit;
  } else {
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
