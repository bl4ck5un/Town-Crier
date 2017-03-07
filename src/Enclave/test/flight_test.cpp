#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "../scrapers/flight.h"
#include "Log.h"

/* Test updated on 2/21/17 */
int flight_self_test(){

  	//LL_NOTICE("USAGE: get_flight_delay(YYYYMMDD, HHmm, flight#, return_variable)\n");
    //LL_NOTICE("\tdate/time in Zulu/UTC, flight in ICAO\n");
    int rc, delay, status;
    
    /* Test1: Test a flight that already departed */
    FlightScraper testScraper;
    flight_error ret = testScraper.get_flight_delay(1487691000, "N800GA", &delay);
    switch(ret){
        case DEPARTED:
            LL_NOTICE("N800GA departed as desired\n");
            break; 
        default:
            LL_NOTICE("N800GA Error, returned %i\n",ret);
            return -1;
    }
    
    /* Test2: Test scraper on a delayed flight */
    switch(testScraper.get_flight_delay(1487685600, "UAL124", &delay)){
        case DELAYED:
            LL_NOTICE("UAL124 delayed by %d seconds\n", delay);
            break;
        default:
            LL_NOTICE("UAL124 error\n");
            return -1;
    }

    /* Test3: Test on invalid input */
    switch(testScraper.get_flight_delay(1477101654960, "UAL124", &delay)){
        case INVALID:
            LL_NOTICE("Invalid Test passed\n");
            break; 
        default:
            LL_NOTICE("Invalid test failed\n");
            return -1;
    }

    /* Test4: Test on a cancelled flight */
    switch(testScraper.get_flight_delay(1487739600, "GCR7531", &delay)){
        case CANCELLED:
            LL_NOTICE("CANCELLED Test passed\n");
            break; 
        default:
            LL_NOTICE("CANCELLED test failed with return code");
            return -1;
    }
    /* Test5: Test on a flight that has left early */
    switch(testScraper.get_flight_delay(1487716380, "JBU2189", &delay)){
        case DEPARTED:
            LL_NOTICE("Flight Departed normally\n");
            break;
        default:
            LL_NOTICE("JBU2189 test error\n");
            return -1;
    } 
    return 0;

    /* Test5: Test on flight that has not departed yet */
}
//1477276620,
//filed_departuretime":1477276620
//filed_departuretime":1477276620
