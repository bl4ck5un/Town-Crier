#ifndef WOLFRAM_H
#define WOLFRAM_H

enum wolfram_error{
	NO_ERROR=0,		/* Produced a valid query */
	INVALID, 		/* Invalid query */
	NO_RESP,		/* Wolfram website irresponsive */
}

class WolframQuery{ 
private:
	std::string query;
	std::string appid;
	/* More stuff can be added here for parsing */
public:
	WolframQuery(std::string query); 

	/* Getter and setter functions */
	void set_query(std::string query)
	void set_appid(std::string appID);
	std::string get_url();
}
class WolframResultParser(){
	WolframResultParser()
}

class WolframScraper : Scraper{
private:
	HttpRequest httpRequest; 
	HttpClient httpClient;

public:
	WolframScraper();
	void WolframScraper::create_query(std::string query);
	err_code handler(uint8_t *req, int data_len, int *resp_data);
	wolfram_error query_website(std::string query);
}


class WolframQueryResult(){
private:
	char* xml; /* The XML response from the wolfram website */
public:
	WolframQueryResult(char* xml);
	char* get_raw_data();
}