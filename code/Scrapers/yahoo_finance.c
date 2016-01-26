#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct str {
    char *ptr;
    size_t len;
};


int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
    size_t new_len = dest->len + size*nmemb;
    dest->ptr = realloc(dest->ptr, new_len+1);
    if (dest->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(dest->ptr+dest->len, ptr, size*nmemb);
    dest->ptr[new_len] = '\0';
    dest->len = new_len;
    return size*nmemb;
}


void init_string(struct str *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

int get_page_on_ssl(const char* url, char* buf, int len) {
    struct str web_return;
    CURL* curl;
    CURLcode res;

    init_string(&web_return);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &web_return);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("curl error: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    int copy_len = (len<web_return.len) ? len : web_return.len;
    memcpy(buf, web_return.ptr, copy_len);
    return copy_len;
}

int construct_query(int argc, char* argv[], char** buf) {
    if (argc != 5) {
        printf("USAGE: %s [month] [day] [year] [symbol]\n", argv[0]);
        return -1;
    }
    
    char* month = argv[1];
    char* day = argv[2];
    char* year = argv[3];
    char* symbol = argv[4];
    char query[100];
    query[0] = 0;

    char* base = "https://ichart.yahoo.com/table.csv?s=";

    // add the symbol to the query
    strcat(query, base);
    strcat(query, symbol);
    
    // yahoo api uses month-1
    int m = atoi(month);
    m = m-1;
    sprintf(month,"%d",m);

    // add dates to query
    strcat(query, "&a=");
    strcat(query, month);
    strcat(query, "&b=");
    strcat(query, day);
    strcat(query, "&c=");
    strcat(query, year);
    strcat(query, "&d=");
    strcat(query, month);
    strcat(query, "&e=");
    strcat(query, day);
    strcat(query, "&f=");
    strcat(query, year);

    // interval (daily)
    strcat(query, "&g=d");

    strcat(query, "&ignore=.csv");
    int len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

int parse_response(char* resp, char** buf) {
    int i;
    char* temp = resp;
    char* end;
    for (i=0; i < 10; i++) {
        while(*temp!=',') {
            temp+= 1;
        }
        temp+=1;
    }
    end = temp;
    while(*end != ',') {
        end+=1;
    }
    //*end = 0;
    int len = end - temp;
    *buf = (char*)malloc(len+1);
    memcpy(*buf, temp, len);
    (*buf)[len] = 0;
    return len;
}

int main(int argc, char* argv[]) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[1001];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(argc, argv, &query);
    if (ret < 0)
        return -1;
    //printf("%s\n", query);

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl(query, buf, 1000); 
    buf[ret] = 0;
    //printf("%s\n", buf);
    
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output);

    /***** OUTPUT */
    printf("closing: %s\n", output);

    return 0;
}
