#include "Enclave.h"
#include "tstdio.h"

#ifndef SCRAPER_LIB_H
#define SCRAPER_LIB_H

#define SSL_MAX_CONTENT_LEN 16384

int get_page_on_ssl(const char* server_name, const char* url, unsigned char* buf, int len);

#endif