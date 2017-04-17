#ifndef STOCKTICKER_H
#define STOCKTICKER_H

#include <string.h>

#include "Scraper.h"
#include "Debug.h"
#include "Log.h"
#include "utils.h"
#include "../../Common/Constants.h"
#include "../external/slre.h"
#include "tls_client.h"

class StockQuery{
private:
	int day;
	int month;
	int year;
	std::string symbol;
public:
	StockQuery(){};
	StockQuery(int day, int month, int year, std::string symbol);
	void SetDay(int day);
	void SetMonth(int month);
	void SetYear(int year); 
	void SetSymbol(std::string symbol);
	std::string GetUrl();
};

class StockTickerParser{
private:
	const char* rawResponse;
	err_code error;
public:
	StockTickerParser(const char* resp, err_code error);
	const char* GetResponse();
	double GetClosingPrice();
	err_code GetErrorCode();
};

class StockTickerScraper : Scraper{
private:
	//HttpRequest httpRequest; 
	//HttpClient httpClient;
	StockQuery query;
public:
	StockTickerScraper() : query() {};
	err_code handler(const uint8_t *req, size_t data_len, int *resp_data);
	void CreateQuery(int date, int month, int year, std::string symbol);
	StockTickerParser QueryWebsite();
};



#endif