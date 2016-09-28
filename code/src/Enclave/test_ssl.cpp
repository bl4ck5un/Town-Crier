//
// Created by fan on 6/16/16.
//

#include <Debug.h>
#include "SSLClient.h"
#include "Scraper_lib.h"
#include "Log.h"

int ssl_test(){
    client_opt_t opt;
    client_opt_init(&opt);
    opt.server_name = "httpbin.org";
    opt.server_port = "443";
    opt.request_page = "/";
    opt.debug_level = 1;
    unsigned char output[1024*100];
    int len;
    int ret = ssl_client(opt, NULL, 0, output, 1024*100, &len);
    if (ret < 0) {
        LL_CRITICAL("Error: returned %d", ret);
        return ret;
    }
    LL_CRITICAL("%d copied", len);
    output[len] = '\0';
    LL_CRITICAL("%d strlen", strlen((const char*)output));

    print_str_dbg("RESPONSE", output, len);
    return ret;
}
