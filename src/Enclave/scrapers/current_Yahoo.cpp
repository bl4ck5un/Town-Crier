#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrapers.h"
#include <string>
#include <Log.h>

#include "tls_client.h"

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


static double parse_response(const char* resp) {
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

    int ret = 0;
    char* query = NULL;

    ret = construct_query(symbol, &query);
    if (ret < 0)
        return -1;

    vector<string> header;
    header.push_back("Host: download.finance.yahoo.com");
    HttpRequest httpRequest("download.finance.yahoo.com", query, header);
    HttpClient httpClient(httpRequest);

    try {
        HttpResponse response = httpClient.getResponse();
        *r  =parse_response(response.getContent().c_str());
        return 0;
    }
    catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError());
        httpClient.close();
    }

    free(query);
    return 0;
}