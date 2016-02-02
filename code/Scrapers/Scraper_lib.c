#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include "Scraper_lib.h"

struct str {
    char *ptr;
    size_t len;
};


int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
    size_t new_len = dest->len + size*nmemb;
    dest->ptr = realloc(dest->ptr, new_len+1);
    if (dest->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(dest->ptr+dest->len, ptr, size*nmemb);
    dest->ptr[new_len] = '\0';
    dest->len = new_len;
    return size*nmemb;
}


void init_string(struct str *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

int get_page_on_ssl(const char* server_name, const char* url, unsigned char* buf, int len) {
    struct str web_return;
    CURL* curl;
    CURLcode res;
    int copy_len;
    struct curl_slist* chunk = NULL;

    init_string(&web_return);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();


    curl_easy_setopt(curl,CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &web_return);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("curl error: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    copy_len = (len<web_return.len) ? len : web_return.len;
    memcpy(buf, web_return.ptr, copy_len);
    return copy_len;
}
