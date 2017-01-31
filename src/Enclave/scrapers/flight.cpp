#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

#include "Debug.h"
#include "Log.h"
#include "scrapers.h"
#include "../../Common/Constants.h"
#include "tls_client.h"

//
//static int utime(const char* ds, const char* ts) {
//    char year[5], month[3], day[3], hour[3], minute[3];
//    int y, mo, d, h, mi, temp;
//    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
//
//    year[4] = 0;
//    month[2] = 0;
//    day[2] = 0;
//    hour[2] = 0;
//    minute[2] = 0;
//
//    memcpy(year, ds, 4);
//    memcpy(month, ds+4, 2);
//    memcpy(day, ds+6, 2);
//    memcpy(hour, ts, 2);
//    memcpy(minute, ts+2, 2);
//    LL_NOTICE("%s %s %s, %s:%s\n", year, month, day, hour, minute);
//    y = atoi(year);
//    mo = atoi(month);
//    d = atoi(day);
//    h = atoi(hour);
//    mi = atoi(minute);
//
//    y = y - 1970;
//    temp = y * 365 * 24 * 60 * 60;
//    temp += ((y/4) * 24 * 60 * 60);
//
//    mo = mo - 2;
//    while (mo >= 0) {
//        temp += mdays[mo] * 24 * 60 * 60;
//        mo--;
//    }
//
//    temp += (d) * 24 * 60 * 60;
//    temp += h * 60 * 60;
//    temp += mi * 60;
//
//    return temp;
//}
//
int construct_query(const char* flight, char** buf) {
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

int parse_response(const char* resp, int* delay, uint64_t unix_epoch_time) {

	//Handle Flight not found
    if(strlen(resp)< 31){
    	*delay = -1; //flight not found
      LL_NOTICE("Flight not found!");
    	return NOT_FOUND;
    }	

    const char* temp  = strchr(resp, '{');
	const char* end;
	const char* sd;

    if (!temp ){
       LL_CRITICAL("Error: buf2 is NULL");
       //*status = INVALID;
       return INVALID;
    }

	int i;
	uint64_t t = unix_epoch_time;

	char tempbuff[100] = {0};
	char tstamp[11] = {0};
	/*struct tm t1;*/
	/*time_t t;*/
	char scheduled[11];
	char actual[11];
	int len, tactual, tscheduled, hours, minutes, seconds, diff;

	snprintf(tstamp, 11, "%llu", t);
	strncpy(tempbuff, "filed_departuretime\":\0", 22);
	strncat(tempbuff, tstamp, sizeof tempbuff);

	len = strlen(resp);

	while(strncmp(temp, tempbuff, sizeof(tempbuff)) != 0) {
	   	temp+=1;
	   	if (temp == resp + len - 32) {
	    	LL_CRITICAL("did not find flight");
	       	return NOT_FOUND;
	   	}
	}

	sd = temp;

	for (i=0; i < 5; i++) {
	   while(*temp!=',') {
	       temp+= 1;
	   }
	   temp+=1;
	}
	temp += 22; //only get up to the next ","
	end = temp;
	while(*end!= ','){
		end += 1;
	}

	//end = temp+10;
	len = end-temp;

	scheduled[len] = 0;
	actual[len] = 0;

	memcpy(actual, temp, len);

	actual[len] = 0;
	tactual = atoi(actual);

//	LL_NOTICE("tactual %d", tactual);

	if (tactual == 0){//Not departured
		LL_NOTICE("Flight not departured");
	   	return NOT_DEPARTURED;
	} 
	else if (tactual == -1){
		LL_NOTICE("Flight Cancelled!");
		return CANCELLED;
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
	if (diff < 0) diff = 0;

	*delay = diff;
	return DEPARTURED;
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
#define AUTH_CODE "Authorization: Basic Y3JvbWFrNDoyYzNiODZiOGM3N2VlYTBjMjRmZjA4OGEyZjU2ZGEyYjk4ZDQwNTQ3"
#define HOST "Host: flightxml.flightaware.com"
///*
//    A few notes on integration
//    - username & password should be passed as an Authorization header field, with Base64(user:password)
//      as its content.
//    - This website is using HTTP 1.1, which requires a Host header field. Otherwise 400.
//*/
int get_flight_delay(uint64_t unix_epoch_time, const char* flight, int* resp) {
    int ret;
    int delay;

    std::vector<string> header;
    header.push_back(AUTH_CODE);
    header.push_back(HOST);

    //Constretruct the query
  std::string query = "/json/FlightXML2/FlightInfoEx?ident=" + std::string(flight) + "&howMany=30&offset=0 HTTP/1.1";

    HttpRequest httpRequest("flightxml.flightaware.com", query, header);
    HttpClient httpClient(httpRequest);

    try{
        HttpResponse response = httpClient.getResponse();
        ret = parse_response(response.getContent().c_str(), &delay, unix_epoch_time);
    }
    catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError());
        httpClient.close();
        return -1;        
    }

    //Handle result of parse_response
  if (ret == INVALID) {
        *resp = ret;
        return -1;
    }
    if (ret == NOT_DEPARTURED){//Indicates that flight has not departured
    	LL_CRITICAL("flight not departured");
        *resp = NOT_DEPARTURED;
    	return 0;
    } 
    else if (ret == NOT_FOUND) {
        LL_CRITICAL("no data/bad request");
        *resp = NOT_FOUND;
        return 0;
    } 
    else if (ret == CANCELLED) {
        LL_CRITICAL("flight cancelled");
        *resp = CANCELLED;
        return 0;
    } 
    else if (ret == DEPARTURED) {
        LL_CRITICAL("flight departured");
        if (delay <= 30) {
            *resp = DEPARTURED;
        }
        else {
           *resp = DELAYED;
        }
        return 0;
    }
    return 0;
}
