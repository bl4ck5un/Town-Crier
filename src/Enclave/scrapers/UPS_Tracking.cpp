#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Log.h>

#include "scrapers.h"
#include "tls_client.h"
#include "../../Common/Constants.h"
// #include <iostream>

using namespace std;

static int construct_query(char* symbol, std::string &query){
    query = "/WebTracking/track?track=yes&trackNums=";
    query += symbol;
    return query.size();
}

static std::string parse_response(const char* resp){

	//char* end;
	char* tmp = (char*)resp; 
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

int ups_tracking (const char* tracking_num, int* status){
	//printf("Begin ups_current\n");
	if (tracking_num == NULL){
		LL_CRITICAL("Error: Passed in NULL Pointer");
		return -1;
	}

	std::string 
	query = "/WebTracking/track?track=yes&trackNums="  + std::string(tracking_num);
	HttpRequest httpRequest("https://wwwapps.ups.com", query);
	HttpClient httpClient(httpRequest);
	std::string result; 
	try{
		HttpResponse response = httpClient.getResponse();
        result = parse_response(response.getContent().c_str());
	}
	catch (std::runtime_error& e){
        LL_CRITICAL("Https error: %s", e.what());
        LL_CRITICAL("Details: %s", httpClient.getError());
        httpClient.close();
        return -1;		
	}

	// return an int according to the result. E.g. 1 for delivered, etc.
    if(result.compare("Package not found") == 0){
    	*status = PACKAGE_NOT_FOUND;
    	return 0;
    }
    if(result.compare("Delivered") == 0){
        *status = DELIVERED;
        return 0;
    }
    if(result.compare("Order processed") == 0){
        *status = ORDER_PROCESSED;
        return 0;
    }
    if(result.compare("Shipped") == 0){
        *status = SHIPPED;
        return 0;
    }
    if(result.compare("In transit") == 0){
        *status = IN_TRANSIT;
        return 0;
    }
    if(result.compare("Out for delivery") == 0){
        *status = OUT_FOR_DELIVERY;
        return 0;
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
