#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "Debug.h"
#include "Log.h"

#include "handlers.h"
#include <cstring>

static int utime(const char* ds, const char* ts) {
    char year[5], month[3], day[3], hour[3], minute[3];
    int y, mo, d, h, mi, temp;
    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    year[4] = 0;
    month[2] = 0;
    day[2] = 0;
    hour[2] = 0;
    minute[2] = 0;

    memcpy(year, ds, 4);
    memcpy(month, ds+4, 2);
    memcpy(day, ds+6, 2);
    memcpy(hour, ts, 2);
    memcpy(minute, ts+2, 2);
    /*printf("%s %s %s, %s:%s\n", year, month, day, hour, minute);*/
    y = atoi(year);
    mo = atoi(month);
    d = atoi(day);
    h = atoi(hour);
    mi = atoi(minute);

    y = y - 1970;
    temp = y * 365 * 24 * 60 * 60;
    temp += ((y/4) * 24 * 60 * 60);

    mo = mo - 2;
    while (mo >= 0) {
        temp += mdays[mo] * 24 * 60 * 60;
        mo--;
    }

    temp += (d-1) * 24 * 60 * 60;
    temp += h * 60 * 60;
    temp += mi * 60;

    return temp;
}

int construct_query(char* flight, char** buf) {
    int len;
    char query[1000];
    
    query[0] = 0;
    // a full test URL
    // http://cromak4:2c3b86b8c77eea0c24ff088a2f56da2b98d40547@flightxml.flightaware.com/json/FlightXML2/FlightInfoEx?ident=DAL900&howMany=30&offset=0
//    strcat(query, "https://cromak4:2c3b86b8c77eea0c24ff088a2f56da2b98d40547@");
    strncat(query, "/json/FlightXML2/FlightInfoEx?ident=", sizeof query);
    strncat(query, flight, sizeof query);
    strncat(query, "&howMany=30&offset=0 HTTP/1.1", sizeof query);

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

int parse_response(char* resp, int* buf, char* date, char* departure) {
    int i, t;
    char* temp = resp;
    char* end;
    char* sd;

    char tempbuff[100] = {0};
    char tstamp[11] = {0};
    /*struct tm t1;*/
    /*time_t t;*/
    char scheduled[11];
    char actual[11];
    int len, tactual, tscheduled, hours, minutes, seconds, diff;

//    tstamp[11] = 0;
    t = utime(date, departure);
    snprintf(tstamp, 11, "%d", t);

    strncpy(tempbuff, "filed_departuretime\":\0", 22);
    strncat(tempbuff, tstamp, sizeof tempbuff);
    len = strlen(resp);
    while(strncmp(temp, tempbuff, 31) != 0) {
        temp+=1;
        if (temp == resp + len - 32) {
            LL_CRITICAL("did not find flight");
            return -1;
        }
    }
    sd = temp;

    for (i=0; i < 7; i++) {
        while(*temp!=',') {
            temp+= 1;
        }
        temp+=1;
    }
    temp += 20;
    end = temp+10;

    scheduled[10] = 0;
    actual[10] = 0;

    len = end-temp;
    memcpy(actual, temp, len);
    actual[len] = 0;
    /*printf("%s\n", actual);*/
    tactual = atoi(actual);
    if (tactual == 0)
        return -1;

    temp = sd;
    for (i=0; i < 2; i++) {
        while(*temp!=',') {
            temp -= 1;
        }
        temp-=1;
    }
    temp -= 8;
    len = 8;
    memcpy(scheduled, temp, len);
    scheduled[len] = 0;
    /*printf("%s\n", scheduled);*/
    scheduled[2] = 0;
    scheduled[5] = 0;
    hours = atoi(scheduled);
    minutes = atoi(scheduled+3);
    seconds = atoi(scheduled+6);
    tscheduled = t + (hours*60*60) + (minutes*60) + seconds;

    diff = (tactual - tscheduled)/60;

    *buf = diff;
    return len;
}

/* 
    date:   YYYYMMDD
    time:   HHmm
    flight: ICAO flight numbers
    resp:   set to the number of minutes late/early

    return: 0 if OK, -1 if no data found or flight still enroute

    date and time in Zulu/UTC
*/
#define AUTH_CODE "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3"
#define HOST "Host: flightxml.flightaware.com"
/*
    A few notes on integration
    - username & password should be passed as an Authorization header field, with Base64(user:password) 
      as its content. 
    - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
*/
int get_flight_delay(char* date, char* time, char* flight, int* resp) {
    /***** VARIABLE DECLARATIONS */
    int ret, delay;
    char buf[20480] = {0};
    char* tmp = NULL;
    char* query = NULL;
    char* headers[] = {AUTH_CODE, HOST};

    ret = construct_query(flight, &query);
    LL_DEBUG("query is %s", query);
    if (ret < 0)
        return -1;

    ret = get_page_on_ssl("flightxml.flightaware.com", query, headers, 2, (unsigned char*)buf, sizeof buf);

    LL_NOTICE("%d bytes returned", strlen(buf));
    tmp  = strchr(buf, '{');

    if (!tmp )
    {
        LL_CRITICAL("Error: buf2 is NULL");
        ret = -1; goto cleanup;
    }

#ifdef VERBOSE
    string_dump("HTTP RESPONSE:", tmp , strlen(tmp ));
#endif 

    free(query);
    ret = parse_response(buf, &delay, date, time);

    if (ret < 0) {
        LL_CRITICAL("no data/bad request");
        ret = -1; goto cleanup;
    }
    else {
        *resp = delay;
        ret = 0;
    }
cleanup:
    return ret;
}

#ifdef MAIN
int main(int argc, char* argv[]) {
    int rc, delay;
    printf("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    printf("\tdate/time in Zulu/UTC, flight in ICAO\n");
    rc = get_flight_delay("20160129", "1450", "DAL900", &delay);
    if (rc < 0)
        printf("Could not find flight info for DAL900 at specified departure time\n");
    else
        printf("Delta Airlines flight 900 is %d minutes late on 26 January 2016 (should be 2 minutes late)\n", delay);


    rc = get_flight_delay("20160204", "0310", "SWA450", &delay);
    printf("%d, %d (should be 11)\n", rc, delay);
    rc = get_flight_delay("20160202", "0650", "UAL1183", &delay);
    printf("%d, %d (should be -12)\n", rc, delay);

    return 0;
}
#endif
