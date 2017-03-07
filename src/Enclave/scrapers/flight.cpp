#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "Scraper.h"
#include "Debug.h"
#include "Log.h"
#include "utils.h"
#include "../../Common/Constants.h"
#include "../external/slre.h"
#include "tls_client.h"


/* Define flight scraper specific constants */

///*
//    A few notes on integration
//    - username & password should be passed as an Authorization header field, with Base64(user:password)
//      as its content.
//    - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
//*/
#define STR1(x)  #x
#define STR(x)  STR1(x)

#define AUTH_CODE "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3"
#define HOST "Host: flightxml.flightaware.com"
#define SECOND_PER_MIN 60       
#define MAX_DELAY_MIN 30
#define NUM_ENTRY 30 
#define HOW_MANY "&howMany=" STR(NUM_ENTRY) 


/* Define flight scraper specific errors */
enum flight_error {
    INVALID=0,          /* Invalid Parameters passed to the function*/
    DEPARTED,           /* The Flight has departed with no delays */
    DELAYED,            /* The flight is delayed */
    CANCELLED,          /* The flight has been cancelled */
    NOT_DEPARTED,       /* The flight has not departed */
    NOT_FOUND,          /* The flight was not found */
    HTTP_ERROR,         /* HTTP request failed */
};

class FlightScraper {
public:

    /*Class used to handle the flight insurance */
    
    /*  The Data is structured as follows:
     *      0x00 - 0x20 string flight_number
     *      0x20 - 0x40 uint64 unix_epoch
     */
    virtual err_code handler(uint8_t *req, int data_len, int *resp_data) {

        //TODO: What does this do?
        if(data_len  != 2*32){
            LL_CRITICAL("Data_len %d*32 is not 2*32", data_len/32);
            return INVALID_PARAMS;
        }
        dump_buf("Request", req, data_len);

        //Parse raw data into valid parameters
        //Can this be done in a more C++ way?
        int ret, delay;
        char flight_number[35] = {0};
        memcpy(flight_number, req, 0x20);


        char *flighttime = (char *) req + 0x20;
        uint64_t unix_epoch = strtol(flighttime, NULL, 10);


        LL_NOTICE("unix_epoch=%ld, flight_number=%s", unix_epoch, flight_number);
        ret = get_flight_delay(unix_epoch, flight_number, &delay);
        LL_NOTICE("get_flight_delay returns %d, delay=%d", ret, delay);

        switch (ret) {
            case INVALID:
                *resp_data = -1;
                return INVALID_PARAMS;

            case NOT_FOUND:
                *resp_data = -1; 
                return INVALID_PARAMS;

            case HTTP_ERROR:
                *resp_data = -1;
                return WEB_ERROR;

            case DEPARTED:
            case DELAYED:
                // delay > 30 return 1
                *resp_data = delay > 30 * 60 ? 1 : 0;
                return NO_ERROR;
            case CANCELLED:
                // FIXME: hardcode 2 to denote cancelled
                *resp_data = 2;
                return NO_ERROR;
            case NOT_DEPARTED:
                // FIXME: hardcode
                *resp_data = 3;
                return NO_ERROR;

        }
    }

    flight_error parse_response(const char *resp, int *delay, uint64_t unix_epoch_time) {
        
        //Find the scheduled departure time
        std::string buff(resp);
        std::string delimeter = "\"filed_departuretime\":" + uint64_to_string(unix_epoch_time);
        std::size_t pos = buff.find(delimeter);

        //Corner Case: Flight was not found
        if (pos > buff.length()){
            return INVALID;
        }

        //Find the actual departure time
        std::string delimeter2 = "actualdeparturetime\":";
        std::size_t pos2 = buff.find(delimeter2, pos);
        std::string token = buff.substr(pos2 + delimeter2.length(),pos2 + delimeter2.length() + 10);
        
        uint64_t actual_depart_time = atoi(token.c_str());

        //Case: Flight has not yet departed
        if (actual_depart_time == 0){
            return NOT_DEPARTED;
        }
        //Case: Flight was cancelled
        if( actual_depart_time == -1){
            return CANCELLED;
        }
        //Case: Flight Departed but delayed
        if(actual_depart_time - unix_epoch_time >= MAX_DELAY_MIN*SECOND_PER_MIN){
            *delay = actual_depart_time - unix_epoch_time;
            return DELAYED;
        }
        //Case: Flight was not delayed
        else{
            *delay = 0;
            return DEPARTED;
        }
    }
//
///*
//    date:   YYYYMMDD
//    time:   HHmm
//    flight: ICAO flight numbers
//    resp:   set to the number of minutes late/early
//
//    return: 0 if OK, -1 if no data found or flight still enroute
//
//    date and time in Zulu/UTC
//*/
    flight_error get_flight_delay(uint64_t unix_epoch_time, const char *flight, int *resp) {

        /* Invalid user input */
        if(flight == NULL || resp == NULL){
            return INVALID;
        }


        //Build header for https request
        std::vector<string> header;
        header.push_back(AUTH_CODE);
        header.push_back(HOST);

        //Construct the query
        std::string query =
                "/json/FlightXML2/FlightInfoEx?ident=" + std::string(flight) + 
                HOW_MANY + 
                "&offset=0 HTTP/1.1";

        HttpRequest httpRequest("flightxml.flightaware.com", query, header);
        HttpClient httpClient(httpRequest);
        flight_error ret;
        try {
            HttpResponse response = httpClient.getResponse();
            ret = parse_response(response.getContent().c_str(), resp, unix_epoch_time);
        }catch (std::runtime_error &e){
            /* An HTTPS error has occured */
            LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError().c_str());
            httpClient.close();
            return HTTP_ERROR;
        }
        return ret;
    }

    std::string uint64_to_string(uint64_t value ){
        // length of 2**64 - 1, +1 for nul.
        char buf[21];
        snprintf(buf, sizeof buf, "%"PRIu64, value);
        std::string str(buf);
        return str;
    }
};

