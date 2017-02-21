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
    std::size_t pos = buf_string.find("temp\":");

    if (pos == std::string::npos)
    {
        return 0.0;
    }
    temp += (pos + 6);
    end = temp;
    while (*end != ',') {
        end += 1;
    }

    ret = std::strtod(temp, NULL);
    return ret;
}

int weather_current(unsigned int zipcode, double* r) {
    /* Null Checker */
    if (zipcode > 99999 || r == NULL){
        LL_CRITICAL("Error: Passed null pointers");
        return -1;
    }
    char tmp_zip[10];
    snprintf(tmp_zip, sizeof(tmp_zip), "%u", zipcode);

    std::string query = "/data/2.5/weather?zip=" +\
                        std::string(tmp_zip) +\
                        ",us";

    HttpRequest httpRequest("api.openweathermap.org", query);
    HttpClient httpClient(httpRequest);

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

