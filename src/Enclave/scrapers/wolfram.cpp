//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TowCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

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
	this->appid = appId;
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

