#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "Log.h"

int ssl_self_test(){
    // XXX compare the length to CURL
    client_opt_t opt;
    client_opt_init(&opt);
    opt.server_name = "httpbin.org";
    opt.server_port = "443";
    opt.request_page = "/get HTTP/1.1";
    opt.debug_level = 0;
    unsigned char output[1024*100];
    int len;
    char* hdrs[] = {"Host: httpbin.org"};
    int ret = ssl_client(opt, hdrs, 1, output, 1024*100, &len);
    if (ret < 0) {
        LL_CRITICAL("Error: returned %d", ret);
        return ret;
    }
    return ret;
}
