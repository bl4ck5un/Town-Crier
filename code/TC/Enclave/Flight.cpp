#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Scraper_lib.h"
#include "Debug.h"
#include "Log.h"

#include "dispatcher.h"

int utime(const char* ds, const char* ts) {
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

/*int main(int argc, char* argv[]) {
    const char* ds = "20160122";
    const char* ts = "1045";
    int rc;
    rc = utime(ds, ts);
    printf("Utime: %d\n", rc);

    return 0;
}*/

int construct_query(char* flight, char* query) {
    int len;
    query[0] = 0;
    strcat(query, "https://cromak4:2c3b86b8c77eea0c24ff088a2f56da2b98d40547@");
    strcat(query, "flightxml.flightaware.com/json/FlightXML2/FlightInfoEx?ident=");
    strcat(query, flight);
    strcat(query, "&howMany=30&offset=0");

    len = strlen(query);
    return len;
}

int parse_response(char* resp, char** buf, char* argv[]) {
    if (!argv)
    {
        return -1;
    }
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
    snprintf(tstamp, 2, "%d", t);

    strncpy(tempbuff, "filed_departuretime\":\0", 100);
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
    snprintf(d, 2, "%d", diff);

    ret[0] = 0;
    /*strcat(ret, "Scheduled: ");
    //strcat(ret, scheduled);
    //strcat(ret, " Actual: ");
    //strcat(ret, actual);*/
    strcat(ret, "Diff: ");
    strcat(ret, d);


    len = strlen(ret);
    *buf = static_cast<char*>(malloc(len+1));
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int flight_scraper() {
    /***** VARIABLE DECLARATIONS */
    int ret;
    char buf[16385];
    char* output = NULL;
    char query[1024];

    /***** CONSTRUCT THE QUERY */
    ret = construct_query("123", query);
    if (ret < 0)
    {
        LL_CRITICAL("construct_query returned %d", ret);
        return -1;
    }

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("flightxml.flightaware.com", query, (unsigned char*)buf, 16384);
    if (ret < 0)
    {
        LL_CRITICAL("get_page_on_ssl returned %d", ret);
        return -1;
    }

    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output, NULL);
    /***** OUTPUT */
    if (ret < 0)
    {
        LL_CRITICAL("no data/bad request");
        return -1;
    }
    
    LL_DEBUG("%s\n", output);
    return 0;
}