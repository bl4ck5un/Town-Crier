#include "Scraper_lib.h"
#include "handlers.h"
#include "Log.h"
#include <string>

// #include <iostream>

using namespace std;

#define PACKAGE_NOT_FOUND 0
#define ORDER_PROCESSED 1
#define SHIPPED 2
#define IN_TRANSIT 3
#define OUT_FOR_DELIVERY 4
#define DELIVERED 5

static int construct_query(char* symbol, std::string &query){
    query = "/WebTracking/track?track=yes&trackNums=";
    query += symbol;
    return query.size();
}

static std::string parse_response(char* resp){
 


	//char* end;
	char* tmp = resp;

	std::string buf_string(resp); 
	//cout << buf_string << "\n";
	std::size_t pos = buf_string.find("id=\"tt_spStatus\"");
	if (pos == std::string::npos){
		std::string no_pkg = "Package not found";
		return no_pkg;
	}
	//std::string new_tmp = tmp.substr(pos, pos + 20);
	//printf("test\n");
	std::size_t start = pos + 41; 
	pos += 41;
	while(tmp[pos] != '\t'){
		pos += 1;
	}
	std::size_t end = pos - 1;
	std::string token = buf_string.substr(start, end-start);

	return token;
}


int ups_tracking (char* tracking_num){
	//printf("Begin ups_current\n");
	int ret = 0;
	int buf_size = 100*1024;
	char* buf = (char*) malloc(buf_size);
	std::string query;

	ret = construct_query(tracking_num, query);
	if(ret < 0){

		//LL_CRITICAL("%s returne %d","construct_query", ret);
		LL_CRITICAL("failure\n");
		return -1;
	}

	/* execute the query*/ 
	ret = get_page_on_ssl("https://wwwapps.ups.com",query.c_str(), NULL ,0,(unsigned char*)buf, buf_size);
    if(ret<0){
        LL_CRITICAL("%s returned %d", "get_page_on_ssl", ret);
        return -1;
    }

    // parse the buffer

    std::string result = parse_response(buf);

	// return an int according to the result. E.g. 1 for delivered, etc.
    if(result.compare("Package not found")==0){
        return PACKAGE_NOT_FOUND;
    }
    if(result.compare("Delivered")==0){
        return DELIVERED;
    }
    if(result.compare("Order processed")==0){
        return ORDER_PROCESSED;
    }
    if(result.compare("Shipped")==0){
        return SHIPPED;
    }
    if(result.compare("In transit")==0){
        return IN_TRANSIT;
    }
    if(result.compare("Out for delivery")==0){
        return OUT_FOR_DELIVERY;
    }
    else{
    	LL_CRITICAL("failed to get information\n");
        return -1;
    }
}

/* Code used for testing
int main(){
	char trackID[] = "1Z1aasdfa581202007873";
	char* tID = trackID;
	std::string status; 
	ups_current(trackID, &status);
	cout << status << "\n";
	return 0; 

}
*/ 
