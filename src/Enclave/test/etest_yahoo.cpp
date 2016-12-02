#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "scrapers/scraper_lib.h"
#include "scrapers.h"
#include "Log.h"


int yahoo_self_test(){

  	//printf("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    //printf("\tdate/time in Zulu/UTC, flight in ICAO\n");
	
	//Null Checker
	double r = 0.0;

	if (yahoo_current(NULL, &r) != -1 || yahoo_current("GOOG", NULL) != -1 ||\
		yahoo_current(NULL, NULL) != -1){
		return -1;
	}

	//Test out regular performance
	
	if(yahoo_current("GOOG", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Yahoo]Goog returns: %f", r);
	
	if (r == 0 || yahoo_current("AAPL", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Yahoo]AAPL returns: %f", r);

	if (r == 0 || yahoo_current("ORCL", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Yahoo]ORCL returns: %f", r);

	if (r == 0 || yahoo_current("YHOO", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Yahoo]YHOO returns: %f", r);


	//Null check
	int ret = yahoo_current("vdbsd", &r);
	if (r != 0.0){
		return -1;
	}
	//stress test
	//for (int i = 0; i < 100; i++){
	//	if (yahoo_current("GOOG", &r) == -1){
	//		return -1;
	//	}
	//}

	return 0;
}
//1477276620,
//filed_departuretime":1477276620
//filed_departuretime":1477276620
