#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "dispatcher.h"
#include <string>
#include <Log.h>

static int construct_query(const char* symbol, std::string& query) {
    query += "/finance?q=";
    query += symbol;
    return query.size();
}


static double parse_response(char* resp) {
    double ret = 0;
    char * end;
    char * temp = resp;

    std::string buf_string(resp);
    std::size_t pos = buf_string.find("itemprop=\"price\"");

    if (pos == std::string::npos)
    {
        return 0.0;
    }

    temp += pos;
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

    ret = std::strtod(temp, NULL);
    return ret;
}

int google_current(const char* symbol, double* r) {

    /* Null Checker */
    if (symbol == NULL || r == NULL){
        LL_CRITICAL("Error: Passed null pointers");
        return -1;
    }
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char* buf = (char*) malloc(300*1024);
    memset(buf, 0, 300*1024);
    std::string query;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(symbol, query);
    if (ret < 0)
    {
        LL_CRITICAL("%s returned %d", "construct_query", ret);
        return -1;
    }

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("google.com", query.c_str(), NULL, 0, (unsigned char*)buf, 300*1024); 
    if (ret < 0)
    {
        LL_CRITICAL("%s returned %d", "get_page_on_ssl", ret);
        return -1;
    }

    /***** PARSE THE RESPONSE */
    *r = parse_response(buf);
    free(buf);
    return 0;
}

//int main(int argc, char* argv[]) {
//    double r;
//    google_current("GOOG", &r);
//    printf("%f\n", r);
//    google_current("YHOO", &r);
//    printf("%f\n", r);
//    return 0;
//}

