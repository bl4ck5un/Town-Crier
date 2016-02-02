#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "time.c"

int construct_query(int argc, char* argv[], char** buf) {
    int len;
    char* flight = argv[3];
    char query[1000];
    if (argc != 4) {
        printf("USAGE: %s [YYYMMDD] [departure time] [flight number]\n", argv[0]);
        printf("\tflight number in ICAO, time in zulu\n");
        return -1;
    }
    
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

int parse_response(char* resp, char** buf, int argc, char* argv[]) {
    int i, t;
    char* temp = resp;
    char* end;
    char* sd;

    char* date = argv[1];
    char* departure = argv[2];

    char tempbuff[100];
    char tstamp[11];
    /*struct tm t1;*/
    /*time_t t;*/
    char scheduled[11];
    char actual[11];
    int len, tactual, tscheduled, hours, minutes, seconds, diff;
    char d[9], ret[100];

    tstamp[10] = 0;
    tempbuff[0] = 0;
    strcat(tempbuff, date);
    strcat(tempbuff, departure);

    /*strptime(tempbuff, "%Y%m%d%H%M", &t1);
    t1.tm_sec = 0;
    t = mktime(&t1);
    t = t - (60*60*5);*/
    /*printf("timestamp: %d\n", (int)t);*/

    t = utime(date, departure);
    sprintf(tstamp, "%d", t);

    strcpy(tempbuff, "filed_departuretime\":\0");
    strcat(tempbuff, tstamp);
    len = strlen(resp);
    while(strncmp(temp, tempbuff, 31) != 0) {
        temp+=1;
        if (temp == resp + len - 32)
            return -1;
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
    sprintf(d, "%d", diff);

    ret[0] = 0;
    /*strcat(ret, "Scheduled: ");
    //strcat(ret, scheduled);
    //strcat(ret, " Actual: ");
    //strcat(ret, actual);*/
    strcat(ret, "Diff: ");
    strcat(ret, d);


    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int main(int argc, char* argv[]) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[16385];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(argc, argv, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("flightxml.flightaware.com", query, (unsigned char*)buf, 16384); 
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output, argc, argv);
    /***** OUTPUT */
    if (ret < 0)
        printf("no data/bad request\n");
    else
        printf("%s\n", output);

    return 0;
}
