#include "eth_ecdsa.h"
#include "external/keccak.h"
#include "Debug.h"
#include "Log.h"
#include "glue.h"
#include "mbedtls/error.h"

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


static int pubkey_to_address (unsigned char *pubkey, size_t pubkey_len, unsigned char* addr)
{
    int ret;
    if (pubkey_len != 64) {
        mbedtls_printf("Error: wrong pubkey length\n");
        return -1;
    }

    ret = keccak(pubkey, pubkey_len, addr, 32);
    return 0;
}


#if defined(VERBOSE)
#else
#define dump_pubkey( a, b )
#define dump_group(a, b)
#define dump_mpi(a, b)
#endif

#define FROM_PRIVATE_KEY "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8"

void keygen(mbedtls_ecdsa_context* ctx)
{
    int ret;
#ifndef FROM_PRIVATE_KEY
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "ecdsa";


    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    mbedtls_printf( "Seeding the random number generator...\n" );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( "Error: mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( "Generating key pair" );

    if( ( ret = mbedtls_ecdsa_genkey(ctx, ECPARAMS,
                              mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        mbedtls_printf( "Error: mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }

    size_t lenSecretKey = mbedtls_mpi_size(&ctx->d);
    mbedtls_ecp_point* pubKey = &ctx->Q;

#else
    mbedtls_ecp_group_load( &ctx->grp, ECPARAMS);
    ret = mbedtls_mpi_read_string(&ctx->d, 16, FROM_PRIVATE_KEY);
    if (ret != 0) {
        mbedtls_printf("Error: mbedtls_mpi_read_string returned %d\n", ret);
        goto exit;
    }
    ret = mbedtls_ecp_mul( &ctx->grp, &ctx->Q, &ctx->d, &ctx->grp.G, NULL, NULL );
    if (ret != 0) {
        mbedtls_printf("Error: mbedtls_ecp_mul returned %d\n", ret);
        goto exit;
    }
    dump_mpi("sk: ", &ctx->d);
#endif
    //mbedtls_printf( "key size: %d bits\n", (int) ctx->grp.pbits );
    //dump_pubkey( "Public key: ", ctx );
    //dump_group("Group used is: \n", & ctx->grp);

exit:
#ifndef FROM_PRIVATE_KEY
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
#endif
    return;
}

int sign(uint8_t* data, int in_len, uint8_t* rr, uint8_t *ss, uint8_t* vv)
{
    int ret;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_mpi r, s;

    // here begins statements
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_ecdsa_init( &ctx_sign );
    mbedtls_ecdsa_init( &ctx_verify );
    mbedtls_ctr_drbg_init( &ctr_drbg );

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
    }
    else {
#ifdef SIGN_DEBUG
        mbedtls_printf("Verified!\n");
#else
        ;
#endif
    }

exit:
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        LL_CRITICAL("Last error was: -0x%X - %s", -ret, error_buf );
    }
    mbedtls_ecdsa_free( &ctx_verify );
    mbedtls_ecdsa_free( &ctx_sign );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    return( ret );
}
