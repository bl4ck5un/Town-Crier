#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"

int construct_query(char* symbol, char** buf) {
    int len;
    char query[1000];
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


int parse_response(char* resp, char** buf) {
    int len;
    char ret[100];

    /*double price;
    
    price = atof(resp);*/
    
    ret[0] = 0;
    strcat(ret, resp);

    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int yahoo_current(char* symbol, double* r) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[16385];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("finance.yahoo.com", query, (unsigned char*)buf, 16384); 
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output);

    /***** OUTPUT */
    /*printf("%s\n", output);*/
    *r = atof(output);

    return 0;
}


int main(int argc, char* argv[]) {
    double r;
    yahoo_current("GOOG", &r);
    printf("%f\n", r);
    yahoo_current("YHOO", &r);
    printf("%f\n", r);

    return 0;
}
