#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scraper_lib.h"

struct str {
    char *ptr;
    size_t len;
};


int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
    size_t new_len = dest->len + size*nmemb;
    dest->ptr = (char*) realloc(dest->ptr, new_len+1);
    if (dest->ptr == NULL) {
        printf("realloc() failed\n");
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
        printf("malloc() failed\n");
    }
    s->ptr[0] = '\0';
}

int construct_query(int argc, char* argv[], char** buf) {
    char* month = argv[1];
    char* day = argv[2];
    char* year = argv[3];
    char* symbol = argv[4];
    char query[100];

    char* base = "ichart.yahoo.com/table.csv?s=";
    int m, len;

    query[0] = 0;

    if (argc != 5) {
        printf("USAGE: %s [month] [day] [year] [symbol]\n", argv[0]);
        return -1;
    }

    // add the symbol to the query
    strcat(query, base);
    strcat(query, symbol);
    
    // yahoo api uses month-1
    m = atoi(month);
    m = m-1;
    
    if (m > 9)
        len = 2;
    else
        len = 1;

    snprintf(month, len + 1, "%d", m);

    // add dates to query
    strcat(query, "&a=");
    strcat(query, month);
    strcat(query, "&b=");
    strcat(query, day);
    strcat(query, "&c=");
    strcat(query, year);
    strcat(query, "&d=");
    strcat(query, month);
    strcat(query, "&e=");
    strcat(query, day);
    strcat(query, "&f=");
    strcat(query, year);

    // interval (daily)
    strcat(query, "&g=d");

    strcat(query, "&ignore=.csv");
    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

int parse_response(unsigned char* resp, char** buf) {
    int i, len;
    unsigned char* temp = resp;
    unsigned char* end;
    
    if (*resp == 0) {
        printf("Buf is empty!\n");
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
    *buf = (char*)malloc(len+1);
    memcpy(*buf, temp, len);
    (*buf)[len] = 0;
    return len;
}

int yahoo_finance(int argc, char* argv[]) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    unsigned char buf[SSL_MAX_CONTENT_LEN]={0};
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(argc, argv, &query);
    if (ret < 0) {
        return -1;
    }
    //printf("%s\n", query);

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("ichart.yahoo.com", query, buf, SSL_MAX_CONTENT_LEN); 
    if (ret != 0){
        printf("get_page_on_ssl returned %d\n", ret);
        return ret;
    }

    //buf[ret] = 0;
    //printf("%s\n", buf);
    
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output);
    if (ret < 0)
        return -1;

    /***** OUTPUT */
    printf("closing: %s\n", output);

    return 0;
}

int test_yahoo_finance ()
{
    char* params[] = {
        "placeholder, delete me later",
        "12",
        "3",
        "2014",
        "BABA"
    };
    return yahoo_finance(5, params);
}
