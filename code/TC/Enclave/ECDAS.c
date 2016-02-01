#include "ECDSA.h"
#include "keccak.h"

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

#include <string.h>
#endif

#include "mbedtls/sha256.h"

/*
 * Uncomment to show key and signature details
 */
#define VERBOSE

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

    keccak(pubkey, pubkey_len, addr, 32);
    return 0;
}


#if defined(VERBOSE)
static void dump_buf( const char *title, unsigned char *buf, size_t len )
{
    size_t i;

    mbedtls_printf( "%s", title );
    for( i = 0; i < len; i++ )
        mbedtls_printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
                       "0123456789ABCDEF" [buf[i] % 16] );
    mbedtls_printf( "\n" );
}

static void dump_pubkey( const char *title, mbedtls_ecdsa_context *key )
{
    // each point on our curve is 256 bit (32 Bytes)
    // two points plus the leading 0x04 byte
    unsigned char buf[2*32 + 1];
    unsigned char addr[32];
    size_t len;

    if( mbedtls_ecp_point_write_binary( &key->grp, &key->Q,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf ) != 0 )
    {
        mbedtls_printf("internal error\n");
        return;
    }

    // buf + 1 to skip the first 0x04 byte
    dump_buf( title, buf + 1, len -1);
    pubkey_to_address(buf + 1, sizeof buf - 1, addr);
    dump_buf( "address: ", addr + 12, 32 - 12);
}

static void dump_mpi (const char* title, mbedtls_mpi* X)
{
    size_t len = mbedtls_mpi_bitlen(X);
    unsigned char* buf;

    if (len == 0)
    {
        printf("%s%d\n", title, 0);
        return;
    }
    
    len = ((len + 7) & ~0x07) / 8;
    buf = (unsigned char*) malloc(len);
    mbedtls_mpi_write_binary (X, buf, len);
    dump_buf (title, buf, len);
    free(buf);
}

static void dump_group( const char* title, mbedtls_ecp_group* grp)
{
    unsigned char buf[128];
    size_t len;

    mbedtls_printf("%s", title);

    dump_mpi("A=", &grp->A);
    dump_mpi("B=", &grp->B);

    mbedtls_ecp_point_write_binary( grp, &grp->G,
                MBEDTLS_ECP_PF_UNCOMPRESSED, &len, buf, sizeof buf );
    dump_buf("G=", buf, len);

    dump_mpi("N=", &grp->N);
    printf("h=%d\n", grp->h);
}
#else
#define dump_buf( a, b, c )
#define dump_pubkey( a, b )
#define dump_group (a, b)
#define dump_mpi (a, b)
#endif


void keygen(mbedtls_ecdsa_context* ctx)
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    int ret;
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

    mbedtls_printf( "key size: %d bits\n", (int) ctx->grp.pbits );
    dump_pubkey( "Public key: ", ctx );
    dump_group("Group used is: \n", & ctx->grp);

exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
}


int test_ecdsa()
{
    int ret;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char hash[32];
    char msg[] = "message";
    unsigned char sig[512];
    size_t sig_len;
    const char *pers = "ecdsa";

    mbedtls_mpi r, s;
    char v;

    // here begins statements

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_ecdsa_init( &ctx_sign );
    mbedtls_ecdsa_init( &ctx_verify );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    memset(sig, 0, sizeof( sig ) );
    ret = 1;

    mbedtls_sha256((unsigned char*) msg, strlen(msg), hash, 0);

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ecdsa_genkey( &ctx_sign, ECPARAMS,
                              mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ecdsa_genkey returned %d\n", ret );
        goto exit;
    }

    dump_pubkey( "pk: ", &ctx_sign );

    // sign
    ret = mbedtls_ecdsa_sign_bitcoin(&ctx_sign.grp, &r, &s, &v, &ctx_sign.d, hash, 32, MBEDTLS_MD_SHA256);
    if (ret != 0) {
        mbedtls_printf("Error: mbedtls_ecdsa_sign_bitcoin returned %d\n", ret);
        goto exit;
    }
    dump_buf("hash: ", hash, 32);
    dump_mpi("r: ", &r);
    dump_mpi("s: ", &s);
    printf  ("v: %d\n", v);

    ret = mbedtls_ecdsa_verify(&ctx_sign.grp, hash, sizeof hash, &ctx_sign.Q, &r, &s);
    if (ret != 0) {
        mbedtls_printf("Error: mbedtls_ecdsa_verify returned %d\n", ret);
    }
    else {
        mbedtls_printf("Verified!\n");
    }

exit:
    mbedtls_ecdsa_free( &ctx_verify );
    mbedtls_ecdsa_free( &ctx_sign );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return( ret );
}