#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ExampleEnclave_t.h"

#ifndef SSL_CLIENT_H
#define SSL_CLIENT_H

#define DFL_SERVER_NAME         NULL
#define DFL_SERVER_ADDR         NULL
#define DFL_SERVER_PORT         "443"
#define DFL_REQUEST_PAGE        "/"
#define DFL_REQUEST_SIZE        -1
#define DFL_DEBUG_LEVEL         0
#define DFL_NBIO                0
#define DFL_READ_TIMEOUT        0
#define DFL_MAX_RESEND          0
#define DFL_CA_FILE             ""
#define DFL_CA_PATH             ""
#define DFL_CRT_FILE            ""
#define DFL_KEY_FILE            ""
#define DFL_PSK                 ""
#define DFL_PSK_IDENTITY        "Client_identity"
#define DFL_ECJPAKE_PW          NULL
#define DFL_FORCE_CIPHER        0
#define DFL_RENEGOTIATION       MBEDTLS_SSL_RENEGOTIATION_DISABLED
#define DFL_ALLOW_LEGACY        -2
#define DFL_RENEGOTIATE         0
#define DFL_EXCHANGES           1
#define DFL_MIN_VERSION         -1
#define DFL_MAX_VERSION         -1
#define DFL_ARC4                -1
#define DFL_AUTH_MODE           -1
#define DFL_MFL_CODE            MBEDTLS_SSL_MAX_FRAG_LEN_NONE
#define DFL_TRUNC_HMAC          -1
#define DFL_RECSPLIT            -1
#define DFL_DHMLEN              -1
#define DFL_RECONNECT           0
#define DFL_RECO_DELAY          0
#define DFL_RECONNECT_HARD      0
#define DFL_TICKETS             MBEDTLS_SSL_SESSION_TICKETS_ENABLED
#define DFL_ALPN_STRING         NULL
#define DFL_TRANSPORT           MBEDTLS_SSL_TRANSPORT_STREAM
#define DFL_HS_TO_MIN           0
#define DFL_HS_TO_MAX           0
#define DFL_FALLBACK            -1
#define DFL_EXTENDED_MS         -1
#define DFL_ETM                 -1

#define GET_REQUEST "GET %s\r\n"
#define GET_REQUEST_END "\r\n"

typedef struct options
{
    const char *server_name;    /* hostname of the server (client only)     */
    const char *server_addr;    /* address of the server (client only)      */
    const char *server_port;    /* port on which the ssl service runs       */
    int debug_level;            /* level of debugging                       */
    int nbio;                   /* should I/O be blocking?                  */
    uint32_t read_timeout;      /* timeout on mbedtls_ssl_read() in milliseconds    */
    int max_resend;             /* DTLS times to resend on read timeout     */
    const char *request_page;   /* page on server to request                */
    int request_size;           /* pad request with header to requested size */
    const char *ca_file;        /* the file with the CA certificate(s)      */
    const char *ca_path;        /* the path with the CA certificate(s) reside */
    const char *crt_file;       /* the file with the client certificate     */
    const char *key_file;       /* the file with the client key             */
    const char *psk;            /* the pre-shared key                       */
    const char *psk_identity;   /* the pre-shared key identity              */
    const char *ecjpake_pw;     /* the EC J-PAKE password                   */
    int force_ciphersuite[2];   /* protocol/ciphersuite to use, or all      */
    int renegotiation;          /* enable / disable renegotiation           */
    int allow_legacy;           /* allow legacy renegotiation               */
    int renegotiate;            /* attempt renegotiation?                   */
    int renego_delay;           /* delay before enforcing renegotiation     */
    int exchanges;              /* number of data exchanges                 */
    int min_version;            /* minimum protocol version accepted        */
    int max_version;            /* maximum protocol version accepted        */
    char arc4;                   /* flag for arc4 suites support             */
    int auth_mode;              /* verify mode for connection               */
    unsigned char mfl_code;     /* code for maximum fragment length         */
    int trunc_hmac;             /* negotiate truncated hmac or not          */
    int recsplit;               /* enable record splitting?                 */
    int dhmlen;                 /* minimum DHM params len in bits           */
    int reconnect;              /* attempt to resume session                */
    int reco_delay;             /* delay in seconds before resuming session */
    int reconnect_hard;         /* unexpectedly reconnect from the same port */
    int tickets;                /* enable / disable session tickets         */
    const char *alpn_string;    /* ALPN supported protocols                 */
    int transport;              /* TLS or DTLS?                             */
    uint32_t hs_to_min;         /* Initial value of DTLS handshake timer    */
    uint32_t hs_to_max;         /* Max value of DTLS handshake timer        */
    char fallback;               /* is this a fallback connection?           */
    char extended_ms;            /* negotiate extended master secret?        */
    char etm;                    /* negotiate encrypt then mac?              */
} client_opt_t;

static void client_opt_init(client_opt_t* opt) {
    opt->server_name         = DFL_SERVER_NAME;
    opt->server_addr         = DFL_SERVER_ADDR;
    opt->server_port         = DFL_SERVER_PORT;
    opt->debug_level         = DFL_DEBUG_LEVEL;
    opt->nbio                = DFL_NBIO;
    opt->read_timeout        = DFL_READ_TIMEOUT;
    opt->max_resend          = DFL_MAX_RESEND;
    opt->request_page        = DFL_REQUEST_PAGE;
    opt->request_size        = DFL_REQUEST_SIZE;
    opt->ca_file             = DFL_CA_FILE;
    opt->ca_path             = DFL_CA_PATH;
    opt->crt_file            = DFL_CRT_FILE;
    opt->key_file            = DFL_KEY_FILE;
    opt->psk                 = DFL_PSK;
    opt->psk_identity        = DFL_PSK_IDENTITY;
    opt->ecjpake_pw          = DFL_ECJPAKE_PW;
    opt->force_ciphersuite[0]= DFL_FORCE_CIPHER;
    opt->renegotiation       = DFL_RENEGOTIATION;
    opt->allow_legacy        = DFL_ALLOW_LEGACY;
    opt->renegotiate         = DFL_RENEGOTIATE;
    opt->exchanges           = DFL_EXCHANGES;
    opt->min_version         = DFL_MIN_VERSION;
    opt->max_version         = DFL_MAX_VERSION;
    opt->arc4                = DFL_ARC4;
    opt->auth_mode           = DFL_AUTH_MODE;
    opt->mfl_code            = DFL_MFL_CODE;
    opt->trunc_hmac          = DFL_TRUNC_HMAC;
    opt->recsplit            = DFL_RECSPLIT;
    opt->dhmlen              = DFL_DHMLEN;
    opt->reconnect           = DFL_RECONNECT;
    opt->reco_delay          = DFL_RECO_DELAY;
    opt->reconnect_hard      = DFL_RECONNECT_HARD;
    opt->tickets             = DFL_TICKETS;
    opt->alpn_string         = DFL_ALPN_STRING;
    opt->transport           = DFL_TRANSPORT;
    opt->hs_to_min           = DFL_HS_TO_MIN;
    opt->hs_to_max           = DFL_HS_TO_MAX;
    opt->fallback            = DFL_FALLBACK;
    opt->extended_ms         = DFL_EXTENDED_MS;
    opt->etm                 = DFL_ETM;
}

#if defined(__cplusplus)
extern "C" {
#endif
int ssl_client(client_opt_t opt, char* headers[], int n_header, unsigned char* buf, int len);

#if defined(__cplusplus)
}
#endif

#endif