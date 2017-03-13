#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <Log.h>

#include "tls_client.h"
#include "scrapers.h"

static double parse_response(const char* resp) {
    double ret = 0;
    const char * end;
    const char * temp = resp;

    std::string buf_string(resp);
    std::size_t pos = buf_string.find("price_usd\": \"");

    if (pos == std::string::npos)
    {
        return 0.0;
    }
    temp += (pos + 13);
    end = temp;
    while (*end != '"') {
        end += 1;
    }

    ret = std::strtod(temp, NULL);
    return ret;
}

int coinmarketcap_current(const char* symbol, double* r) {
    /* Null Checker */
    if (symbol == NULL || r == NULL){
        LL_CRITICAL("Error: Passed null pointers");
        return -1;
    }

    std::string query = "/ticker/" + std::string(symbol) + "/";
    HttpRequest httpRequest("api.coinmarketcap.com", query);
    HttpsClient httpClient(httpRequest);

    try {
        HttpResponse response = httpClient.getResponse();
        *r = parse_response(response.getContent().c_str());
        return 0;
    }
    catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
        httpClient.close();
    }

    return 0;
}

