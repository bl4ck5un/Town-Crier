#include <stdio.h>
#include <stdlib.h>
#include <Debug.h>

#include "tls_client.h"
#include "scrapers.h"
#include "Log.h"


int coin_self_test(){
	//Null Checker
	double r = 0.0;
	

	if (coinmarketcap_current(NULL, &r) != -1 || coinmarketcap_current("GOOG", NULL) != -1 ||\
		coinmarketcap_current(NULL, NULL) != -1){
		return -1;
	}

	//Test out regular performance
	if(coinmarketcap_current("bitcoin", &r) == -1){
		return -1;
	}
	LL_INFO("[CoinMarket] bitcoin returns: %f", r);
	
	if (r == 0 || coinmarketcap_current("litecoin", &r) == -1){
		return -1;
	}
	LL_INFO("[CoinMarket] litecoin returns: %f", r);

	if (r == 0 || coinmarketcap_current("ethereum", &r) == -1){
		return -1;
	}
	LL_INFO("[CoinMarket] ethereum returns: %f", r);

	if (r == 0 || coinmarketcap_current("ripple", &r) == -1){
		return -1;
	}
	LL_INFO("[CoinMarket] ripple returns: %f", r);


	//Null check
	int ret = coinmarketcap_current("vdbsd", &r);
	if (r != 0.0){
		return -1;
	}

	return 0;
}