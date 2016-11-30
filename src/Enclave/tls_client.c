#include "tls_client.h"
#include "Log.h"
#include "Enclave_t.h"
#include "trusted_ca_certs.h"
#include "Debug.h"
#include "external/http_parser.h"
#include "../Common/Constants.h"
#include "scrapers/scraper_lib.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/platform.h"

#include "mbedtls/net_v.h"
#include "mbedtls/net_f.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    const char *p, *basename;
    (void)(ctx);

    /* Extract basename from file */
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;

    mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str );
}

/*
 * Test recv/send functions that make sure each try returns
 * WANT_READ/WANT_WRITE at least once before sucesseding
 */
static int my_recv( void *ctx, unsigned char *buf, size_t len )
{
    static int first_try = 1;
    int ret;

    if( first_try )
    {
        first_try = 0;
        return( MBEDTLS_ERR_SSL_WANT_READ );
    }

    ret = mbedtls_net_recv( ctx, buf, len );
    if( ret != MBEDTLS_ERR_SSL_WANT_READ )
        first_try = 1; /* Next call will be a new operation */
    return( ret );
}

static int my_send( void *ctx, const unsigned char *buf, size_t len )
{
    static int first_try = 1;
    int ret;

    if( first_try )
    {
        first_try = 0;
        return( MBEDTLS_ERR_SSL_WANT_WRITE );
    }

    ret = mbedtls_net_send( ctx, buf, len );
    if( ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        first_try = 1; /* Next call will be a new operation */
    return( ret );
}

#if defined(MBEDTLS_X509_CRT_PARSE_C)
/*
 * Enabled if debug_level > 1 in code below
 */
static int my_verify( void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags )
{
    char buf[1024];
    ((void) data);

    LL_LOG( "\nVerify requested for (Depth %d):", depth );
    mbedtls_x509_crt_info( buf, sizeof( buf ) - 1, "", crt );
    LL_LOG( "%s", buf );

    if ( ( *flags ) == 0 )
        LL_LOG( "  This certificate has no flags" );
    else
    {
        mbedtls_x509_crt_verify_info( buf, sizeof( buf ), "  ! ", *flags );
        LL_LOG( "%s", buf );
    }

    return( 0 );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */

typedef struct {
    uint8_t eof;
    uint8_t* buffer;
    int* copied;
} cb_data_t;

int cb_on_message_complete (http_parser* parser) {
    LL_LOG("message_complete");
    cb_data_t* d = (cb_data_t*) parser->data;
    d->eof = 1;
    return 0;
}

int cb_on_body (http_parser* parser, const char* at, size_t len) {
    LL_LOG("On body called with at=%p and len=%d", at, len);
    cb_data_t* p = (cb_data_t*) parser->data;

    uint8_t* dest = p->buffer + *p->copied;
    memcpy(dest, at, len);
    *p->copied += len;

    return 0;
}

int ssl_client(client_opt_t opt, const char* headers[], int n_header, unsigned char* output, int length, int* copied)
{
    int ret = 0, len, tail_len, i, written, frags, retry_left;
    mbedtls_net_context server_fd;
    unsigned char buf[MBEDTLS_SSL_MAX_CONTENT_LEN + 1];

#if defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED)
    unsigned char psk[MBEDTLS_PSK_MAX_LEN];
    size_t psk_len = 0;
#endif
#if defined(MBEDTLS_SSL_ALPN)
    const char *alpn_list[10];
#endif
    const char *pers = "TownCrier";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ssl_session saved_session;
#if defined(MBEDTLS_TIMING_C)
    mbedtls_timing_delay_context timer;
#endif
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    uint32_t flags;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
#endif
    char *p;
    const int *list;

    /*
     * Make sure memory references are valid.
     */
    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    memset( &saved_session, 0, sizeof( mbedtls_ssl_session ) );
    mbedtls_ctr_drbg_init( &ctr_drbg );
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_init( &cacert );
    mbedtls_x509_crt_init( &clicert );
    mbedtls_pk_init( &pkey );
#endif
#if defined(MBEDTLS_SSL_ALPN)
    memset( (void * ) alpn_list, 0, sizeof( alpn_list ) );
#endif

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( opt.debug_level );
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        LL_CRITICAL(" mbedtls_ctr_drbg_seed returned -%#x", -ret);
        goto exit;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    /*
     * 1.1. Load the trusted CA
     */

    // load trusted crts
    ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) root_cas_pem, root_cas_pem_len);
    if( ret < 0 )
    {
        LL_CRITICAL("  mbedtls_x509_crt_parse returned -%#x", -ret);
        goto exit;
    }
#endif /* MBEDTLS_X509_CRT_PARSE_C */

    /*
     * 2. Start the connection
     */
    if( opt.server_addr == NULL)
        opt.server_addr = opt.server_name;

    LL_LOG("connecting over %s: %s:%s...",
            opt.transport == MBEDTLS_SSL_TRANSPORT_STREAM ? "TCP" : "UDP",
            opt.server_addr, opt.server_port );

    if( ( ret = mbedtls_net_connect( &server_fd, opt.server_addr, opt.server_port,
                             opt.transport == MBEDTLS_SSL_TRANSPORT_STREAM ?
                             MBEDTLS_NET_PROTO_TCP : MBEDTLS_NET_PROTO_UDP ) ) != 0 )
    {
        LL_CRITICAL( " mbedtls_net_connect returned -%#x", -ret );
        goto exit;
    }

    if( opt.nbio > 0 )
        ret = mbedtls_net_set_nonblock( &server_fd );
    else
        ret = mbedtls_net_set_block( &server_fd );
    if( ret != 0 )
    {
        LL_CRITICAL( " net_set_(non)block() returned -%#x", -ret );
        goto exit;
    }

    /*
     * 3. Setup stuff
     */
    LL_LOG( "Setting up the SSL/TLS structure..." );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    opt.transport,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        LL_CRITICAL( "mbedtls_ssl_config_defaults returned -%#x", -ret );
        goto exit;
    }

    /*
    if( opt.debug_level > 0 )
        mbedtls_ssl_conf_verify( &conf, my_verify, NULL );
    */

    if( opt.auth_mode != DFL_AUTH_MODE )
        mbedtls_ssl_conf_authmode( &conf, opt.auth_mode );

#if defined(MBEDTLS_SSL_PROTO_DTLS)
    if( opt.hs_to_min != DFL_HS_TO_MIN || opt.hs_to_max != DFL_HS_TO_MAX )
        mbedtls_ssl_conf_handshake_timeout( &conf, opt.hs_to_min, opt.hs_to_max );
#endif /* MBEDTLS_SSL_PROTO_DTLS */

#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH)
    if( ( ret = mbedtls_ssl_conf_max_frag_len( &conf, opt.mfl_code ) ) != 0 )
    {
        mbedtls_printf( "  mbedtls_ssl_conf_max_frag_len returned %d\n\n", ret );
        goto exit;
    }
#endif

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, NULL );

    mbedtls_ssl_conf_read_timeout( &conf, opt.read_timeout );
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);

#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_conf_session_tickets( &conf, opt.tickets );
#endif

    if( opt.force_ciphersuite[0] != DFL_FORCE_CIPHER )
        mbedtls_ssl_conf_ciphersuites( &conf, opt.force_ciphersuite );

#if defined(MBEDTLS_ARC4_C)
    if( opt.arc4 != DFL_ARC4 )
        mbedtls_ssl_conf_arc4_support( &conf, opt.arc4 );
#endif

    if( opt.allow_legacy != DFL_ALLOW_LEGACY )
        mbedtls_ssl_conf_legacy_renegotiation( &conf, opt.allow_legacy );
#if defined(MBEDTLS_SSL_RENEGOTIATION)
    mbedtls_ssl_conf_renegotiation( &conf, opt.renegotiation );
#endif

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        LL_CRITICAL("mbedtls_ssl_setup returned -%#x", -ret );
        goto exit;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if( ( ret = mbedtls_ssl_set_hostname( &ssl, opt.server_name ) ) != 0 )
    {
        LL_CRITICAL("mbedtls_ssl_set_hostname returned %d\n\n", ret );
        goto exit;
    }
#endif

    if( opt.nbio == 2 )
        mbedtls_ssl_set_bio( &ssl, &server_fd, my_send, my_recv, NULL );
    else
        mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv,
                             opt.nbio == 0 ? mbedtls_net_recv_timeout : NULL );

#if defined(MBEDTLS_TIMING_C)
    mbedtls_ssl_set_timer_cb( &ssl, &timer, mbedtls_timing_set_delay,
                                            mbedtls_timing_get_delay );
#endif

    /*
     * 4. Handshake
     */
    LL_LOG( "Performing the SSL/TLS handshake" );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            LL_CRITICAL( "mbedtls_ssl_handshake returned -%#x", -ret );
            if( ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED )
                LL_CRITICAL(
                    "Unable to verify the server's certificate. "
                    "Either it is invalid,"
                    "or you didn't set ca_file or ca_path "
                    "to an appropriate value."
                    "Alternatively, you may want to use "
                    "auth_mode=optional for testing purposes." );
            goto exit;
        }
    }

    LL_LOG( "Hand shake succeeds: [%s, %s]",
            mbedtls_ssl_get_version( &ssl ), mbedtls_ssl_get_ciphersuite( &ssl ) );

    if( ( ret = mbedtls_ssl_get_record_expansion( &ssl ) ) >= 0 )
        LL_DEBUG( "Record expansion is [%d]", ret );
    else
        LL_DEBUG( "Record expansion is [unknown (compression)]" );

#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH)
    LL_LOG( "Maximum fragment length is [%u]",
                    (unsigned int) mbedtls_ssl_get_max_frag_len( &ssl ) );
#endif


#if defined(MBEDTLS_X509_CRT_PARSE_C)
    /*
     * 5. Verify the server certificate
     */
    LL_LOG( "Verifying peer X.509 certificate..." );

    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];
        mbedtls_printf( " failed\n" );
        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
        mbedtls_printf( "%s\n", vrfy_buf );
    }
    else 
    {
        LL_LOG("X.509 Verifies");
    }

    if( mbedtls_ssl_get_peer_cert( &ssl ) != NULL )
    {
        if (opt.debug_level > 0)
        {
            LL_DEBUG( "Peer certificate information");
            mbedtls_x509_crt_info( (char *) buf, sizeof( buf ) - 1, "|-", mbedtls_ssl_get_peer_cert( &ssl ) );
            LL_DEBUG("%s\n", buf);
        }

    }
#endif /* MBEDTLS_X509_CRT_PARSE_C */

    /*
     * 6. Write the GET request
     */
    retry_left = opt.max_resend;

send_request:
    len = mbedtls_snprintf( (char *) buf, sizeof(buf) - 1, GET_REQUEST, opt.request_page );

    if (headers && n_header > 0)
    {
        for (i = 0; i < n_header; i++)
        {
            len += mbedtls_snprintf( (char*)buf + len, sizeof(buf) -1 - len, "%s\r\n", headers[i]);
        }
    }
    tail_len = (int) strlen( GET_REQUEST_END );

    /* Add padding to GET request to reach opt.request_size in length */
    if( opt.request_size != DFL_REQUEST_SIZE &&
        len + tail_len < opt.request_size )
    {
        memset( buf + len, 'A', opt.request_size - len - tail_len );
        len += opt.request_size - len - tail_len;
    }

    strncpy( (char *) buf + len, GET_REQUEST_END, sizeof(buf) - len - 1 );
    len += tail_len;

    /* Truncate if request size is smaller than the "natural" size */
    if( opt.request_size != DFL_REQUEST_SIZE &&
        len > opt.request_size )
    {
        len = opt.request_size;

        /* Still end with \r\n unless that's really not possible */
        if( len >= 2 ) buf[len - 2] = '\r';
        if( len >= 1 ) buf[len - 1] = '\n';
    }

    if( opt.transport == MBEDTLS_SSL_TRANSPORT_STREAM )
    {
        for( written = 0, frags = 0; written < len; written += ret, frags++ )
        {
            while( ( ret = mbedtls_ssl_write( &ssl, buf + written, len - written ) )
                           <= 0 )
            {
                if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
                    ret != MBEDTLS_ERR_SSL_WANT_WRITE )
                {
                    mbedtls_printf( "  mbedtls_ssl_write returned -%#x", -ret );
                    goto exit;
                }
            }
        }
    }

    buf[written] = '\0';

    if (opt.debug_level > 0) {
        hexdump("REQUEST:", buf, written);
    }

    /*
     * 7. Read the HTTP response
     */

    /*
     * TLS and DTLS need different reading styles (stream vs datagram)
     */

    // create a HTTP parser
    http_parser* parser = (http_parser*) malloc(sizeof(http_parser));
    if (parser == NULL) {
        LL_CRITICAL("failed to malloc %s", "http_parser");
        ret = ERR_ENCLAVE_SSL_CLIENT;
        goto exit;
    }
    http_parser_init(parser, HTTP_RESPONSE);

    http_parser_settings settings;
    http_parser_settings_init(&settings);

    size_t nparsed;

    // prepare callback data
    cb_data_t* p_cb_data = (cb_data_t*) malloc(sizeof (cb_data_t));
    if (!p_cb_data) {
        LL_CRITICAL("Error: can't malloc for p_cb_data (%d)", sizeof(cb_data_t));
        ret = ERR_ENCLAVE_SSL_CLIENT;
        goto exit;
    }
    memset(p_cb_data, 0, sizeof(cb_data_t));

    p_cb_data->buffer = output;
    p_cb_data->copied = copied;
    *copied = 0;

    parser->data = p_cb_data;

    settings.on_message_complete = cb_on_message_complete;
    settings.on_body = cb_on_body;

    if( opt.transport == MBEDTLS_SSL_TRANSPORT_STREAM )
    {
        do
        {
            len = sizeof(buf) - 1;
            memset(buf, 0, sizeof(buf));
            /*
            return the number of bytes read, or 0 for EOF, or
            MBEDTLS_ERR_SSL_WANT_READ or MBEDTLS_ERR_SSL_WANT_WRITE, or
            MBEDTLS_ERR_SSL_CLIENT_RECONNECT (see below), or another negative
            error code.
            */
            ret = mbedtls_ssl_read(&ssl, buf, len);

            if( ret == MBEDTLS_ERR_SSL_WANT_READ ||
                ret == MBEDTLS_ERR_SSL_WANT_WRITE )
                continue;

            if( ret < 0 )
            {
                switch( ret )
                {
                    case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                        LL_CRITICAL( " connection was closed gracefully" );
                        ret = 0;
                        goto close_notify;
                    case MBEDTLS_ERR_NET_CONN_RESET:
                        LL_CRITICAL( " connection was reset by peer" );
                        ret = 0;
                        goto reconnect;

                    default:
                        LL_CRITICAL( " mbedtls_ssl_read returned -0x%x", -ret );
                        goto exit;
                }
            }

            if (*copied + ret > length) {
                LL_CRITICAL("Output buffer is not big enough (%d)", length);
                ret = ERR_ENCLAVE_SSL_CLIENT;
                goto exit;
            }

            len = ret;
            LL_LOG("%d bytes received.", ret);
            
            if (opt.debug_level > 0) {
                hexdump("RESPONSE", buf, ret);
            }

            nparsed = http_parser_execute(parser, &settings, buf, len);
            LL_LOG("%d bytes parsed", nparsed);
            LL_LOG("%d bytes copied", *copied);
            if (parser->upgrade) {
                LL_CRITICAL("Not supprted yet");
                ret = ERR_ENCLAVE_SSL_CLIENT;
                goto exit;
            } else if (nparsed != len) {
                LL_CRITICAL("Error happend");
                ret = ERR_ENCLAVE_SSL_CLIENT;
                goto exit;
            }


            if (p_cb_data->eof == 1) {
                LL_LOG("EOF");
                break;
            }
        }
        while( 1 );
    }
    else /* Not stream, so datagram */
    {
        LL_CRITICAL("please use TCP in opt.transport which is %d", opt.transport);
        ret = -1;
        goto exit;
    }

    /*
     * 7b. Simulate hard reset and reconnect from same port?
     */
    if( opt.reconnect_hard != 0 )
    {
        opt.reconnect_hard = 0;

        mbedtls_printf( "  . Restarting connection from same port..." );

        if( ( ret = mbedtls_ssl_session_reset( &ssl ) ) != 0 )
        {
            mbedtls_printf( "  mbedtls_ssl_session_reset returned -%#x", -ret );
            goto exit;
        }

        while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
        {
            if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
                ret != MBEDTLS_ERR_SSL_WANT_WRITE )
            {
                mbedtls_printf( "  mbedtls_ssl_handshake returned -%#x", -ret );
                goto exit;
            }
        }

        mbedtls_printf( " ok\n" );

        goto send_request;
    }

    /*
     * 7c. Continue doing data exchanges?
     */
    if( --opt.exchanges > 0 )
        goto send_request;

    /*
     * 8. Done, cleanly close the connection
     */
close_notify:

    /* No error checking, the connection might be closed already */
    do ret = mbedtls_ssl_close_notify( &ssl );
    while( ret == MBEDTLS_ERR_SSL_WANT_WRITE );
    ret = 0;

    LL_LOG( "closed %s:%s", opt.server_addr, opt.server_port );

    /*
     * 9. Reconnect?
     */
reconnect:
    if( opt.reconnect != 0 )
    {
        --opt.reconnect;

        mbedtls_net_free( &server_fd );

#if defined(MBEDTLS_TIMING_C)
        if( opt.reco_delay > 0 )
            mbedtls_net_usleep( 1000000 * opt.reco_delay );
#endif

        mbedtls_printf( "  . Reconnecting with saved session..." );

        if( ( ret = mbedtls_ssl_session_reset( &ssl ) ) != 0 )
        {
            mbedtls_printf( "  mbedtls_ssl_session_reset returned -%#x", -ret );
            goto exit;
        }

        if( ( ret = mbedtls_ssl_set_session( &ssl, &saved_session ) ) != 0 )
        {
            mbedtls_printf( "  mbedtls_ssl_conf_session returned %d\n\n", ret );
            goto exit;
        }

        if( ( ret = mbedtls_net_connect( &server_fd, opt.server_addr, opt.server_port,
                                 opt.transport == MBEDTLS_SSL_TRANSPORT_STREAM ?
                                 MBEDTLS_NET_PROTO_TCP : MBEDTLS_NET_PROTO_UDP ) ) != 0 )
        {
            mbedtls_printf( "  mbedtls_net_connect returned -%#x", -ret );
            goto exit;
        }

        if( opt.nbio > 0 )
            ret = mbedtls_net_set_nonblock( &server_fd );
        else
            ret = mbedtls_net_set_block( &server_fd );
        if( ret != 0 )
        {
            mbedtls_printf( "  net_set_(non)block() returned -%#x",
                    -ret );
            goto exit;
        }

        while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
        {
            if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
                ret != MBEDTLS_ERR_SSL_WANT_WRITE )
            {
                mbedtls_printf( "  mbedtls_ssl_handshake returned -%#x", -ret );
                goto exit;
            }
        }

        mbedtls_printf( " ok\n" );

        goto send_request;
    }

    /*
     * Cleanup and exit
     */
exit:
#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        LL_CRITICAL("Last error was: -0x%X - %s\n\n", -ret, error_buf );
    }
#endif

    mbedtls_net_free( &server_fd );

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_free( &clicert );
    mbedtls_x509_crt_free( &cacert );
    mbedtls_pk_free( &pkey );
#endif
    mbedtls_ssl_session_free( &saved_session );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    // Shell can not handle large exit numbers -> 1 for errors

    return( ret );
}
