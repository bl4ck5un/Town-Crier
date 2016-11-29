#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "dispatcher.h"
#include "Log.h"


int google_self_test(){

  	//printf("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    //printf("\tdate/time in Zulu/UTC, flight in ICAO\n");
	
	//Null Checker
	double r = 0.0;
	if (google_current(NULL, &r) != -1 || google_current("GOOG", NULL) != -1 ||\
		google_current(NULL, NULL) != -1){
		return -1;
	}

	//Test out regular performance
	
	if(google_current("GOOG", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google]Goog returns: %f\n", r);
	
	if (r == 0 || google_current("Apple", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google]Apple returns: %f\n", r);

	if (r == 0 || google_current("oracle", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google]oracle returns: %f\n", r);

	if (r == 0 || google_current("YAHOO", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google]YAHOO returns: %f\n", r);


	//Null check
	int ret = google_current("vdbsd", &r);
	if (r != 0.0){
		return -1;
	}
	//stress test
	//for (int i = 0; i < 100; i++){
	//	if (google_current("GOOG", &r) == -1){
	//		return -1;
	//	}
	//}

	return 0;
}
//1477276620,
//filed_departuretime":1477276620
//filed_departuretime":1477276620
