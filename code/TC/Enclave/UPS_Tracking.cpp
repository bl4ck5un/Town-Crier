#include "Scraper_lib.h"
#include "handlers.h"
#include "Log.h"
#include <string>

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

int ups_tracking (char* tracking_num, const char* status){
	//printf("Begin ups_current\n");
	int ret = 0;
	int buf_size = 100*1024;
	char* buf = (char*) malloc(buf_size);
	std::string query;

	ret = construct_query(tracking_num, query);
	if(ret < 0){
		LL_CRITICAL("%s returned %d","construct_query", ret);
		return -1;
	}

	/* execute the query*/ 
	ret = get_page_on_ssl("https://wwwapps.ups.com",query.c_str(), NULL ,0,(unsigned char*)buf, buf_size);
    if(ret<0){
        LL_CRITICAL("%s returned %d", "get_page_on_ssl", ret);
        return -1;
    }

    // parse the buffer
    std::string tmp_string = parse_response(buf);

    status = tmp_string.c_str();

    return 0;
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