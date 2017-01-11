#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>
#include "tls_client.h"
#include "scrapers.h"
#include "Log.h"


int google_self_test(){
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
	LL_NOTICE("[Google] Goog returns: %f", r);
	
	if (r == 0 || google_current("Apple", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google] Apple returns: %f", r);

	if (r == 0 || google_current("oracle", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google] oracle returns: %f", r);

	if (r == 0 || google_current("YAHOO", &r) == -1){
		return -1;
	}
	LL_NOTICE("[Google] YAHOO returns: %f", r);


	//Null check
	int ret = google_current("vdbsd", &r);
	if (r != 0.0){
		return -1;
	}

	return 0;
}