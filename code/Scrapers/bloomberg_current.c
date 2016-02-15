#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"

int construct_query(char* symbol, char** buf) {
    int len;
    char query[1000];
    query[0] = 0;

    strcat(query, "https://www.bloomberg.com/quote/");
    strcat(query, symbol);
    strcat(query, ":US");

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}


int parse_response(char* resp, char** buf) {
    int len;
    char ret[100];
    char * end;
    char * temp = resp;

    while (strncmp(temp, "itemprop=\"price\"", 16) != 0) {
        temp += 1;
    }
    temp += 17;
    while (*temp != '"') {
        temp += 1;
    }
    temp += 1;
    end = temp;
    while (*end != '"') {
        end += 1;
    }
    *end = 0;


    /*double price;
    
    price = atof(resp);*/
    
    ret[0] = 0;
    strcat(ret, temp);

    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int bloomberg_current(char* symbol, double* r) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[300000];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("bloomberg.com", query, (unsigned char*)buf, 299999); 
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
    bloomberg_current("GOOG", &r);
    printf("%f\n", r);
    bloomberg_current("YHOO", &r);
    printf("%f\n", r);
    return 0;
}
