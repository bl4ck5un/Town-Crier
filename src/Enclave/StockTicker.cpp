#include <string>

#include "Scraper_lib.h"
#include "dispatcher.h"
#include "stdio.h"
#include "Log.h"

using namespace std;

struct str {
    char *ptr;
    size_t len;
};


int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
    size_t new_len = dest->len + size*nmemb;
    dest->ptr = (char*) realloc(dest->ptr, new_len+1);
    if (dest->ptr == NULL) {
        LL_CRITICAL("realloc() failed\n");
        return -1;
    }
    memcpy(dest->ptr+dest->len, ptr, size*nmemb);
    dest->ptr[new_len] = '\0';
    dest->len = new_len;
    return size*nmemb;
}


void init_string(struct str *s) {
    s->len = 0;
    s->ptr = (char*) malloc(s->len+1);
    if (s->ptr == NULL) {
        LL_CRITICAL("malloc() failed\n");
    }
    s->ptr[0] = '\0';
}

int construct_query(int month, int day, int year, const char* symbol, char* query, int len) {
    snprintf(query, len, 
        "ichart.yahoo.com/table.csv?s=%s&a=%d&b=%d&c=%d&d=%d&e=%d&f=%d&g=d&ignore=.csv",
        symbol, month-1, day, year, month-1, day, year);
    return 0;
}

int parse_response(unsigned char* resp, char* buf) {
    int i, len;
    unsigned char* temp = resp;
    unsigned char* end;
    
    if (*resp == 0) {
        LL_CRITICAL("Buf is empty!\n");
        return -1;
    }
    for (i=0; i < 10; i++) {
        while(*temp!=',') {
            temp+= 1;
        }
        temp+=1;
    }
    end = temp;
    while(*end != ',') {
        end+=1;
    }
    //*end = 0;
    len = end - temp;
    memcpy(buf, temp, len);
    buf[len] = 0;
    return len;
}

double get_closing_price(int month, int day, int year, const char* symbol) {
    int ret;
    unsigned char buf[SSL_MAX_CONTENT_LEN]={0};
    char query[128] = {0};
    char result[128] = {0};

    ret = construct_query(month, day, year, symbol, query, sizeof query);
    if (ret < 0) {
        return -1;
    }

    ret = get_page_on_ssl("ichart.yahoo.com", query, NULL, 0, buf, sizeof buf); 
    if (ret != 0){
        LL_CRITICAL("get_page_on_ssl returned %d\n", ret);
        return ret;
    }

    ret = parse_response(buf, result);
    if (ret < 0)
        return -1;

    return std::strtod(result, NULL);
}
