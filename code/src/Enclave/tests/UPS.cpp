#include <Debug.h>
#include "Scraper_lib.h"
#include "Log.h"
#include "UPS_Tracking.h"

int Google_scraper_test(){ 
	//test normal performance of the google scraper
	double r;

	//Case: Test Parsing on a delivered package

	char delivered_track_ID[] = "1ZE331480394808282";
	char unknown_track_ID[] = "1234567890";
	//TODO: Need more track ids to track package in other states

	if(UPS_Tracking(delivered_track_ID) != DELIVERED){
		LL_CRITICAL("UPS TRACKER FAILED");
		return -1;
	}
	if(UPS_Tracking(unknown_track_ID) !=PACKAGE_NOT_FOUND){
		LL_CRITICAL("UPS TRACKER FAILED");
		return -1;
	}
	//Stress Tests: Query the same trackID 10000 times
	for (int i = 0; i < 10000; i++){
		if (PS_Tracking(delivered_track_ID) != DELIVERED){
			LL_CRITICAL("google scraper failed stress test");
			return -1;
		}
	}
	//Passed all tests!
	return 0;

}