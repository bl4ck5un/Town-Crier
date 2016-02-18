#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "dispatcher.h"

static int construct_query(char* symbol, char** buf) {
    int len;
    char query[1000];
    query[0] = 0;

    strncat(query, "/quote/", sizeof query);
    strncat(query, symbol, sizeof query);
    strncat(query, ":US", sizeof query);

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}


static int parse_response(char* resp, char** buf) {
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
    strncat(ret, temp, sizeof ret);

    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int bloomberg_current(double* r) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[3000];
    char* query = NULL;
    char* output = NULL;

    char* symbol = "GOOG";

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("bloomberg.com", query, NULL, 0, (unsigned char*)buf, sizeof buf); 
    /*printf("%s\n", buf);*/

    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output);

    /***** OUTPUT */
    /*printf("%s\n", output);*/
    *r = atof(output);

    return 0;
}

//int main(int argc, char* argv[]) {
//    double r;
//    bloomberg_current("GOOG", &r);
//    printf("%f\n", r);
//    bloomberg_current("YHOO", &r);
//    printf("%f\n", r);
//    return 0;
//}
