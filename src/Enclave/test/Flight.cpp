#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "dispatcher.h"
#include "Log.h"

//extern "C" int flight_self_test();

int flight_self_test(){

  	//printf("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    //printf("\tdate/time in Zulu/UTC, flight in ICAO\n");
    int rc, delay, status;
    rc = get_flight_delay(1477114200, "SOL361", &delay);
    if (rc < 0){
        //printf("Could not find flight info for DAL900 at specified departure time\n");
    	return -1;
    }
    
    if(delay != 2147483647){
    	LL_NOTICE("canceled failed\n");
    	return -1;
        //printf("Delta Airlines flight 900 is %d minutes late on 26 January 2016 (should be 2 minutes late)\n", delay);
    }


    //rc = get_flight_delay("20160204", "0310", "SWA450", &delay);
    //printf("%d, %d (should be 11)\n", rc, delay);
    //rc = get_flight_delay("20160202", "0650", "UAL1183", &delay);
    //printf("%d, %d (should be -12)\n", rc, delay);


    //Test2: Test on jargon/dummy inputs
    rc = get_flight_delay(1477101654960, "SOL", &delay);
    if (status != 2){
    	LL_NOTICE("jargon failed");
    	return -1;
    }
    //if (delay != -1){
    //	return -1;
    //}
    
    //Test3: Test on departured flight
    rc = get_flight_delay(1477226100, "ASH6110", &delay);
    if (status != 0){
    	LL_NOTICE("departured failed\n");
    	return -1;
    }
    //Test4: Test on valid, not departured flight
    rc = get_flight_delay(1477398900, "ASH6110", &delay);
    if (status != 1){
    	LL_NOTICE("not departured failed\n");
    	return -1;
    }

	return 0;


}
//1477276620,
//filed_departuretime":1477276620
//filed_departuretime":1477276620
