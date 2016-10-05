#include "Enclave.h"

#ifndef SCRAPER_LIB_H
#define SCRAPER_LIB_H

#define SSL_MAX_CONTENT_LEN 16384

#if defined(__cplusplus)
extern "C" {
#endif
int get_page_on_ssl(const char* server_name, const char* url, char* header[], int n_header, unsigned char* buf, int len);
#if defined(__cplusplus)
}
#endif
#endif