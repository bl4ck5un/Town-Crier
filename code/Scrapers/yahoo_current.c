#define _GNU_SOURCE
#define __USE_XOPEN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Scraper_lib.h"

int construct_query(int argc, char* argv[], char** buf) {
    int len;
    char* symbol;
    char query[1000];
    if (argc != 2) {
        printf("USAGE: %s [symbol]\n", argv[0]);
        return -1;
    }
    symbol = argv[1];
    query[0] = 0;

    strcat(query, "https://download.finance.yahoo.com/d/quotes.csv?s=");
    strcat(query, symbol);
    strcat(query, "&f=l1");

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}


int parse_response(char* resp, char** buf, int argc, char* argv[]) {
    int len;
    char ret[100];

    /*double price;
    
    price = atof(resp);*/
    
    resp[strlen(resp)-1] = 0;
    strcpy(ret, "Latest price: ");
    strcat(ret, resp);

    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int main(int argc, char* argv[]) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[16385];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(argc, argv, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("finance.yahoo.com", query, (unsigned char*)buf, 16384); 
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output, argc, argv);

    /***** OUTPUT */
    printf("%s\n", output);

    return 0;
}
