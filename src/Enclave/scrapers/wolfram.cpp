/* Define wolfrasm scraper constant */

#define APPID "A8V8R2-523WY42ULW"

/*** Implement the WolframQuery class ***/

/* Constructor for the WolframQuery class */
WolframQuery::WolframQuery(std::string query){
	this->query = query;
	this->appid = APPID; /* Make this a constant */
}
void WolframQuery::set_query(std::string query){
	this->query = query;
}
void WolframQuery::set_appid(std::string appId){
	this->appid = appId
}
std::string WolframQuery::get_url(){
	std::string tmp = "/v2/input=" + this->query + "&appid=" + this->appid;
	return tmp;
}

/*** Implement the WolframQueryResults class ***/

/* WolframResults constructor */
WolframQueryResult::WolframQueryResult(char* xml){
	this->xml = xml;
}

/* Return a pointer to the raw xml file */
char* WolframQueryResult::get_raw_data(){
	return this->xml;
}

/*** Implement the WolfRamScraper class ***/
void WolframScraper::create_query(std::string query){
	WolframQuery wolframQuery(query);
	this->httpRequest = new HttpRequest("api.wolframalpha.com",query.get_url());
	this-> httpClient = new HttpClient(httpRequest);
}
/* Function that performs the HTTPS request and return the xml file */
WolframQueryResult WolframScraper::perform_query(){
	wolfram_error ret;
	try{
		HttpResponse resp = httpClient.getResponse();
	}
	catch(std::runtime_error &e){
		LL_CRITICAL("Https error: %s", e.what());
    	LL_CRITICAL("Details: %s", httpClient.getError().c_str());
    	httpClient.close();
    	return NO_RESP;
	}
	WolframQueryResult wolframQueryResult(resp.c_str());
	/* Do something with these results */
}

