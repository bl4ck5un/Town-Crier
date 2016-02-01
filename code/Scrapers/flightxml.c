#define _GNU_SOURCE
#define __USE_XOPEN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>

struct str {
    char *ptr;
    size_t len;
};


int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
    size_t new_len = dest->len + size*nmemb;
    dest->ptr = (char*)realloc(dest->ptr, new_len+1);
    if (dest->ptr == NULL) {
        printf("realloc() failed\n");
        return -1;
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
        printf("malloc() failed\n");
    }
    s->ptr[0] = '\0';
}

int get_page_on_ssl(const char* server_name, const char* url, char* buf, int len) {
    struct str web_return;
    /*struct curl_slist* chunk = NULL;*/
    CURL* curl;
    CURLcode res;
    int copy_len;

    init_string(&web_return);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();


    curl_easy_setopt(curl,CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &web_return);
    /*curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);*/

    /*FILE* headerfile = fopen("hfile.txt", "wb");
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, headerfile);*/

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("curl error: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    copy_len = (len<web_return.len) ? len : web_return.len;
    memcpy(buf, web_return.ptr, copy_len);
    return copy_len;
}

int construct_query(int argc, char* argv[], char** buf) {
    int len;
    char* flight = argv[2];
    char query[1000];
    if (argc != 6) {
        printf("USAGE: %s [YYYMMDD] [flight number] [origin] [destination] [departure time]\n", argv[0]);
        printf("\torigin/destination airport codes in ICAO, time in zulu\n");
        return -1;
    }
    
    query[0] = 0;

    strcat(query, "https://cromak4:2c3b86b8c77eea0c24ff088a2f56da2b98d40547@");
    strcat(query, "flightxml.flightaware.com/json/FlightXML2/FlightInfoEx?ident=");
    strcat(query, flight);
    strcat(query, "&howMany=30&offset=0");

    len = strlen(query);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, query, len);
    (*buf)[len] = 0;
    return len;
}

/*format: HH:MMxx (xx = AM/PM)*/
int time_diff(char* t1, char* t2) {
    int h1, h2, m1, m2;
    char temp[10];
    strcpy(temp, t1);
    temp[2] = 0;
    h1 = atoi(temp);
    if (temp[5] == 'P')
        h1 += 12;
    temp[5] = 0;
    m1 = atoi(temp+3);

    strcpy(temp, t2);
    temp[2] = 0;
    h2 = atoi(temp);
    if (temp[5] == 'P')
        h2 += 12;
    temp[5] = 0;
    m2 = atoi(temp+3);

    m1 = m1 - m2;
    h1 = h1 - h2;
    h1 = h1*60 + m1;
    return h1;
}

int parse_response(char* resp, char** buf, int argc, char* argv[]) {
    int i;
    char* temp = resp;
    char* end;
    char* sd;

    char* date = argv[1];
    char* departure = argv[5];

    char tempbuff[100];
    char tstamp[11];
    struct tm t1;
    time_t t;
    char scheduled[11];
    char actual[11];
    int len, tactual, tscheduled, hours, minutes, seconds, diff;
    char d[9], ret[100];

    tstamp[10] = 0;
    tempbuff[0] = 0;
    strcat(tempbuff, date);
    strcat(tempbuff, departure);

    strptime(tempbuff, "%Y%m%d%H%M", &t1);
    t1.tm_sec = 0;
    t = mktime(&t1);
    t = t - (60*60*5);
    /*printf("timestamp: %d\n", (int)t);*/
    sprintf(tstamp, "%d", (int)t);

    strcpy(tempbuff, "filed_departuretime\":\0");
    strcat(tempbuff, tstamp);
    while(strncmp(temp, tempbuff, 31) != 0) {
        temp+=1;
    }
    sd = temp;

    for (i=0; i < 7; i++) {
        while(*temp!=',') {
            temp+= 1;
        }
        temp+=1;
    }
    temp += 20;
    end = temp+10;

    scheduled[10] = 0;
    actual[10] = 0;

    len = end-temp;
    memcpy(actual, temp, len);
    actual[len] = 0;
    /*printf("%s\n", actual);*/
    tactual = atoi(actual);

    temp = sd;
    for (i=0; i < 2; i++) {
        while(*temp!=',') {
            temp -= 1;
        }
        temp-=1;
    }
    temp -= 8;
    len = 8;
    memcpy(scheduled, temp, len);
    scheduled[len] = 0;
    /*printf("%s\n", scheduled);*/
    scheduled[2] = 0;
    scheduled[5] = 0;
    hours = atoi(scheduled);
    minutes = atoi(scheduled+3);
    seconds = atoi(scheduled+6);
    tscheduled = t + (hours*60*60) + (minutes*60) + seconds;

    diff = (tactual - tscheduled)/60;
    sprintf(d, "%d", diff);

    ret[0] = 0;
    /*strcat(ret, "Scheduled: ");
    //strcat(ret, scheduled);
    //strcat(ret, " Actual: ");
    //strcat(ret, actual);*/
    strcat(ret, "Diff: ");
    strcat(ret, d);


    len = strlen(ret);
    *buf = (char*)malloc(len+1);
    memcpy(*buf, ret, len);
    (*buf)[len] = 0;
    return len;
}

int main(int argc, char* argv[]) {
    /***** VARIABLE DECLARATIONS */
    int ret = 0;
    char buf[40001];
    char* query = NULL;
    char* output = NULL;

    /***** CONSTRUCT THE QUERY */
    ret = construct_query(argc, argv, &query);
    if (ret < 0)
        return -1;
    /*printf("%s\n", query);*/

    /***** EXECUTE THE QUERY */
    ret = get_page_on_ssl("flightxml.flightaware.com", query, buf, 16384); 
    buf[ret] = 0;
    /*printf("%s\n", buf);*/
    /***** PARSE THE RESPONSE */
    ret = parse_response(buf, &output, argc, argv);

    /***** OUTPUT */
    printf("%s\n", output);

    return 0;
}
