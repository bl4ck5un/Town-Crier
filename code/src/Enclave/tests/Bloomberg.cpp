#include <Debug.h>
#include "Scraper_lib.h"
#include "Log.h"
#include "Current_bloomberg.h"

int Bloomberg_scraper_test(){
	//test normal performance of the google scraper
	double r;
	bloomberg_current("GOOG", &r);
	bloomberg_current("APPLE", &r);
	bloomberg_current("YHOO", &r);

	//Stress Tests: Query the same page 10000 times
	for (int i = 0; i < 10000; i++){
		if (bloomberg_current("GOOG", &r) != 0){
			LL_CRITICAL("bloomberg scraper failed stress test");
		}
	}

	//Corner Case: Query something that doesnt exists
	int ret = bloomberg_current("QWERTY", &r);
	if (ret != -1) {
		LL_CRITICAL("QWERTY has returned %d instead of error");
	}

	return 0;

}