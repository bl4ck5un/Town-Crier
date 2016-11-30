#ifndef SCRAPER_LIB_H
#define SCRAPER_LIB_H

#include "tls_client.h"

#define SSL_MAX_CONTENT_LEN 16384

#if defined(__cplusplus)
extern "C" {
#endif

int get_page_on_ssl(const char* server_name, const char* url, const char* header[], int n_header, unsigned char* buf, int len);
void client_opt_init(client_opt_t* opt);

#if defined(__cplusplus)
}
#endif
#endif
