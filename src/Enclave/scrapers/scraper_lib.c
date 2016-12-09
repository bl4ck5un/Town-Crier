#include "tls_client.h"
#include "scraper_lib.h"

void client_opt_init(client_opt_t* opt) {
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

int get_page_on_ssl(const char* server_name, const char* url, const char* headers[], int n_header, unsigned char* buf, int len)
{

    client_opt_t opt;
    client_opt_init(&opt);

    opt.request_page    = url;
    opt.server_name     = server_name;
    opt.server_port     = "443";
    opt.debug_level     = 0;

    int read = 0;

    return ssl_client(opt, headers, n_header, buf, len, &read);
}
