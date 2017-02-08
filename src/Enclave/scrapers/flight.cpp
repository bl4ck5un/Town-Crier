#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "Debug.h"
#include "Log.h"
#include "utils.h"
#include "scrapers.h"
#include "../../Common/Constants.h"
#include "tls_client.h"

#define AUTH_CODE "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3"
#define HOST "Host: flightxml.flightaware.com"

/* Define flight scraper specific errors */
enum flight_error{
    INVALID=0,      /* Invalid Parameters passed to the function*/
    DEPARTED,       /* The Flight has departed */
    DELAYED,        /* The flight is delayed */
    CANCELLED,      /* The flight has been cancelled */
    NOT_DEPARTED,   /* The flight has not departed */
    NOT_FOUND,       /* The flight was not found */
    HTTP_ERROR,      /* HTTP request failed */
};

class FlightScraper : public Scraper {
public:
    /*Class used to handle a flight insurance */
/*
 *  0x00 - 0x20 string flight_number
 *  ox20 - 0x40 uint64 unix_epoch
 */
    virtual err_code handler(uint8_t *req, int len, int *resp_data) {

        //TODO: What does this do?
        dump_buf("Request", req, len);

        int ret, delay;
        char flight_number[35] = {0};
        memcpy(flight_number, req, 0x20);

        char *flighttime = (char *) req + 0x20;
        uint64_t unix_epoch = strtol(flighttime, NULL, 10);

        LL_NOTICE("unix_epoch=%ld, flight_number=%s", unix_epoch, flight_number);
        switch (get_flight_delay(unix_epoch, flight_number, &delay)) {
            case INVALID:
                return INVALID_PARAMS;
            case HTTP_ERROR:
                return WEB_ERROR;
            case DEPARTED:
                return NO_ERROR;
            case DELAYED:
                return NO_ERROR;
            case CANCELLED:
                return NO_ERROR;
            case NOT_DEPARTED:
                return NO_ERROR;
            case NOT_FOUND:
                return NO_ERROR;
        }

        LL_NOTICE("delay is %d", delay);

        *resp_data = delay;
        return ret;
    }

private:
    /* Helper class used to parse responses return the delay of the thing*/
    flight_error parse_response(const char *resp, int *delay, uint64_t unix_epoch_time) {

        //Handle Flight not found
        if (strlen(resp) < 31) {
            *delay = -1; //flight not found
            LL_NOTICE("Flight not found!");
            return NOT_FOUND;
        }

        const char *temp = strchr(resp, '{');
        const char *end;
        const char *sd;

        if (!temp) {
            LL_CRITICAL("Error: buf2 is NULL");
            //*status = INVALID;
            return INVALID;
        }

        int i;
        uint64_t t = unix_epoch_time;

        char tempbuff[100] = {0};
        char tstamp[11] = {0};
        char scheduled[11];
        char actual[11];
        int len, tactual, tscheduled, hours, minutes, seconds, diff;

        snprintf(tstamp, 11, "%llu", t);
        strncpy(tempbuff, "filed_departuretime\":\0", 22);
        strncat(tempbuff, tstamp, sizeof tempbuff);

        len = strlen(resp);

        while (strncmp(temp, tempbuff, sizeof(tempbuff)) != 0) {
            temp += 1;
            if (temp == resp + len - 32) {
                LL_CRITICAL("did not find flight");
                return NOT_FOUND;
            }
        }

        sd = temp;

        for (i = 0; i < 5; i++) {
            while (*temp != ',') {
                temp += 1;
            }
            temp += 1;
        }
        temp += 22; //only get up to the next ","
        end = temp;
        while (*end != ',') {
            end += 1;
        }

        //end = temp+10;
        len = end - temp;

        scheduled[len] = 0;
        actual[len] = 0;

        memcpy(actual, temp, len);

        actual[len] = 0;
        tactual = atoi(actual);

//	LL_NOTICE("tactual %d", tactual);

        if (tactual == 0) {//Not departured
            LL_NOTICE("Flight not departured");
            return NOT_DEPARTED;
        } else if (tactual == -1) {
            LL_NOTICE("Flight Cancelled!");
            return CANCELLED;
        }
        temp = sd;
        for (i = 0; i < 2; i++) {
            while (*temp != ',') {
                temp -= 1;
            }
            temp -= 1;
        }
        temp -= 8;
        len = 8;

        memcpy(scheduled, temp, len);
        scheduled[len] = 0;
        /*printf("%s\n", scheduled);*/
        scheduled[2] = 0;
        scheduled[5] = 0;
        hours = atoi(scheduled);
        minutes = atoi(scheduled + 3);
        seconds = atoi(scheduled + 6);
        tscheduled = t + (hours * 60 * 60) + (minutes * 60) + seconds;

        diff = (tactual - tscheduled) / 60;
        if (diff < 0) diff = 0;

        *delay = diff;
        return DEPARTED;
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


///*
//    A few notes on integration
//    - username & password should be passed as an Authorization header field, with Base64(user:password)
//      as its content.
//    - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
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
                "/json/FlightXML2/FlightInfoEx?ident=" + std::string(flight) + "&howMany=30&offset=0 HTTP/1.1";

        HttpRequest httpRequest("flightxml.flightaware.com", query, header);
        HttpClient httpClient(httpRequest);
        int ret, delay;
        try {
            HttpResponse response = httpClient.getResponse();
            ret = parse_response(response.getContent().c_str(), &delay, unix_epoch_time);
        }catch (std::runtime_error &e){
            /* An HTTPS error has occured */
            LL_CRITICAL("Https error: %s", e.what());
            LL_CRITICAL("Details: %s", httpClient.getError());
            httpClient.close();
            return HTTP_ERROR;
        }

        switch (ret){
            case INVALID:
                return -1;
            case NOT_DEPARTED:
                return -1;
            case DEPARTED:
                return -1;
            case CANCELLED:
                return -1;
            case NOT_FOUND:
                return -1;
        }
        //Handle result of parse_response
        if (ret == INVALID) {
            *resp = ret;
            return -1;
        }
        if (ret == NOT_DEPARTED) {//Indicates that flight has not departured
            LL_CRITICAL("flight not departured");
            *resp = NOT_DEPARTED;
            return 0;
        } else if (ret == NOT_FOUND) {
            LL_CRITICAL("no data/bad request");
            *resp = NOT_FOUND;
            return 0;
        } else if (ret == CANCELLED) {
            LL_CRITICAL("flight cancelled");
            *resp = CANCELLED;
            return 0;
        } else if (ret == DEPARTED) {
            LL_CRITICAL("flight departured");
            if (delay <= 30) {
                *resp = DEPARTED;
            } else {
                *resp = DELAYED;
            }
            return 0;
        }
        return 0;
    }
};

