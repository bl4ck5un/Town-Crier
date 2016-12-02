#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scraper_lib.h"
#include "scrapers.h"
#include <string>
#include <Log.h>

static int construct_query(const char* symbol, char** buf) {
    int len;
    char query[1000];
    query[0] = 0;

    strncat(query, "/d/quotes.csv?s=", sizeof query);
    strncat(query, symbol, sizeof query);
    strncat(query, "&f=l1", sizeof query);
    strncat(query, " HTTP/1.1", sizeof query);

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}


static double parse_response(char* resp) {
    int len;
    len = strlen(resp);
    /// FIXME PLEASE ADD SOME ERROR HANDLING
    //if (len > 8)
    //{
    //    return 0.0;
    //}
    //else
    //{
    return std::strtod(resp, NULL);
    //}
}

int yahoo_current(const char* symbol, double* r) {

    if (symbol == NULL || r == NULL){
        LL_CRITICAL("Error: Passed in NULL pointer args");
        return -1;
    }
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[16385];
    char* query = NULL;

    const char*headers[] = {"Host: download.finance.yahoo.com"};

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("download.finance.yahoo.com", query, headers, 1, (unsigned char*)buf, 16384);
    free(query);
    /*printf("%s\n", buf);*/
    //LL_CRITICAL("%s\n", buf);
    /***** PARSE THE RESPONSE */
    *r = parse_response(buf);
    return 0;
}

//
//int main(int argc, char* argv[]) {
//    double r;
//    yahoo_current("GOOG", &r);
//    printf("%f\n", r);
//    yahoo_current("YHOO", &r);
//    printf("%f\n", r);
//
//    return 0;
//}
