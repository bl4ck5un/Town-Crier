#include <Debug.h>
#include "Scraper_lib.h"
#include "Log.h"
#include "Current_Google.h"

int Google_scraper_test(){ 
	//test normal performance of the google scraper
	double r;
	google_current("GOOG", &r);
	google_current("APPLE", &r);
	google_current("YHOO", &r);

	//Stress Tests: Query the same page 10000 times
	for (int i = 0; i < 10000; i++){
		if (google_current("GOOG", &r) != 0){
			LL_CRITICAL("google scraper failed stress test");
		}
	}

	//Corner Case: Query something that doesnt exists
	int ret = google_current("QWERTY", &r);
	if (ret != -1) {
		LL_CRITICAL("QWERTY has returned %d instead of error");
	}
	//Corner Case: Company that exists but does not have any information
	// on google's finance page (different page in google)
	int ret = google_current("EXIT", &r);
	if (ret != -1){
		LL_CRITICAL("EXIT has returned with a value of %d", ret);
	}

	return 0;

}