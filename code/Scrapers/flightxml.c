#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "utime.h"

int construct_query(char* flight, char** buf) {
    int len;
    char query[1000];
    
    query[0] = 0;

    strcat(query, "https://cromak4:2c3b86b8c77eea0c24ff088a2f56da2b98d40547@");
    strcat(query, "flightxml.flightaware.com/json/FlightXML2/FlightInfoEx?ident=");
    strcat(query, flight);
    strcat(query, "&howMany=30&offset=0");

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

    char tempbuff[100];
    char tstamp[11];
    /*struct tm t1;*/
    /*time_t t;*/
    char scheduled[11];
    char actual[11];
    int len, tactual, tscheduled, hours, minutes, seconds, diff;


    tstamp[10] = 0;
    t = utime(date, departure);
    snprintf(tstamp, 11, "%d", t);

    strncpy(tempbuff, "filed_departuretime\":\0", 22);
    strcat(tempbuff, tstamp);
    len = strlen(resp);
    while(strncmp(temp, tempbuff, 31) != 0) {
        temp+=1;
        if (temp == resp + len - 32) {
            printf("did not find flight\n");
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
    if (tactual == 0) {
        /*printf("Flight still enroute");*/
        return -1;
    }

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
int get_flight_delay(char* date, char* time, char* flight, int* resp) {
    /***** VARIABLE DECLARATIONS */
    int ret, delay;
    char buf[16385];
    char* query = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(flight, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("flightxml.flightaware.com", query, (unsigned char*)buf, 16384); 
    free(query);
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &delay, date, time);
    /***** OUTPUT */
    if (ret < 0) {
        printf("no data/bad request\n");
        return -1;
    }
    else {
        /*printf("%d\n", delay);*/
        *resp = delay;
        return 0;
    }
}


int main(int argc, char* argv[]) {
    int rc, delay;
    printf("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    printf("\tdate/time in Zulu/UTC, flight in ICAO\n");
    rc = get_flight_delay("20160214", "1455", "DAL900", &delay);
    if (rc < 0)
        printf("Could not find flight info for DAL900 at specified departure time\n");
    else
        printf("Delta Airlines flight 900 is %d minutes late on 14 February 2016 (should be 25 minutes late)\n", delay);


    rc = get_flight_delay("20160212", "2200", "SWA450", &delay);
    printf("%d, %d (should be -10)\n", rc, delay);
    rc = get_flight_delay("20160215", "0655", "UAL1183", &delay);
    printf("%d, %d (should be -5)\n", rc, delay);

    return 0;
}

