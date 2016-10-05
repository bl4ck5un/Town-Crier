#include <curl/curl.h>
#include <string>
#include <cstring>
#include <iostream>

#include "https.h"

using namespace std;

static size_t callback(void* body, size_t size, size_t nmemb, void* buf)
{
    size_t bytes = size * nmemb;
    ((string*)buf)->append((char*)body, bytes);
    return bytes;
}


int get_page_on_ssl(const char* server_name, const char* url, char* headers[],
        int n_header, unsigned char* buf, int len)
{
  CURL *curl;
  CURLcode res;

  string full_url = string(server_name) + string(url);

  cout << "Fetching URL: " << full_url << endl;

  string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.ups.com/");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;

    curl_easy_cleanup(curl);
  }

  strncpy((char* )buf, readBuffer.c_str(), len);

  curl_global_cleanup();

  return 0;
}
