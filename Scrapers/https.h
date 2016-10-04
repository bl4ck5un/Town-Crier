#ifndef HTTPS_H
#define HTTPS_H
int get_page_on_ssl(const char* server_name, const char* url, 
        char* headers[], int n_header, 
        unsigned char* buf, int len);
#endif
