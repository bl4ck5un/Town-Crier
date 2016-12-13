#include <Debug.h>
#include "Scraper_lib.h"
#include "Log.h"
#include "Current_Google.h"

int Yahoo_scraper_test(){
	//test normal performance of the google scraper
	double r;
	yahoo_current("GOOG", &r);
	yahoo_current("APPLE", &r);
	yahoo_current("YHOO", &r);

	//Stress Tests: Query the same page 10000 times
	for (int i = 0; i < 10000; i++){
		if (yahoo_current("GOOG", &r) != 0){
			LL_CRITICAL("yahoo scraper failed stress test");
		
	}

	//Corner Case: Query something that doesnt exists
	int ret = yahoo_current("QWERTY", &r);
	if (ret != -1) {
		LL_CRITICAL("QWERTY has returned %d instead of error");
	}

	return 0;

}