#ifndef WOLFRAM_H
#define WOLFRAM_H

#include <string>
#include "Scraper.h"
#include "utils.h"
enum wolfram_error{
	No_ERR=0,		/* Produced a valid query */
	INVALID, 		/* Invalid query */
	NO_RESP,		/* Wolfram website irresponsive */
};

class WolframQueryResult{
private:
	std::string xml; /* The XML response from the wolfram website */
public:
	WolframQueryResult(std::string xml);
	std::string get_raw_data();
};

class WolframScraper : Scraper {
private:
	const std::string HOST = "api.wolframalpha.com";
	const std::string APPID = "A8V8R2-523WY42ULW";
	std::string url;

public:
	enum WolframQuery{
		SIMPLE=0, 	/* Request simple 1 line answers */
		GENERAL		/* Request more general responses */
	} wolframQueryType;

	//WolframScraper(int qtype);
	void create_query(std::string query);
	err_code handler(const uint8_t *req, size_t data_len, int *resp_data);

	WolframQueryResult perform_query();
	void set_qtype(int type);
};

#endif
