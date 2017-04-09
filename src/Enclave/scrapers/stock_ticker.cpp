//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

//#include <string>
//
//#include "scraper_lib.h"
//#include "scrapers.h"
//#include "stdio.h"
//#include "Log.h"
//
//using namespace std;
//
//struct str {
//    char *ptr;
//    size_t len;
//};
//
//
//int writefunc(char* ptr, size_t size, size_t nmemb, struct str* dest){
//    size_t new_len = dest->len + size*nmemb;
//    dest->ptr = (char*) realloc(dest->ptr, new_len+1);
//    if (dest->ptr == NULL) {
//        LL_CRITICAL("realloc() failed\n");
//        return -1;
//    }
//    memcpy(dest->ptr+dest->len, ptr, size*nmemb);
//    dest->ptr[new_len] = '\0';
//    dest->len = new_len;
//    return size*nmemb;
//}
//
//
//void init_string(struct str *s) {
//    s->len = 0;
//    s->ptr = (char*) malloc(s->len+1);
//    if (s->ptr == NULL) {
//        LL_CRITICAL("malloc() failed\n");
//    }
//    s->ptr[0] = '\0';
//}
//
//int construct_query(int month, int day, int year, const char* symbol, char* query, int len) {
//    snprintf(query, len,
//        "ichart.yahoo.com/table.csv?s=%s&a=%d&b=%d&c=%d&d=%d&e=%d&f=%d&g=d&ignore=.csv",
//        symbol, month-1, day, year, month-1, day, year);
//    return 0;
//}
//
//int parse_response(unsigned char* resp, char* buf) {
//    int i, len;
//    unsigned char* temp = resp;
//    unsigned char* end;
//
//    if (*resp == 0) {
//        LL_CRITICAL("Buf is empty!\n");
//        return -1;
//    }
//    for (i=0; i < 10; i++) {
//        while(*temp!=',') {
//            temp+= 1;
//        }
//        temp+=1;
//    }
//    end = temp;
//    while(*end != ',') {
//        end+=1;
//    }
//    //*end = 0;
//    len = end - temp;
//    memcpy(buf, temp, len);
//    buf[len] = 0;
//    return len;
//}
//
double get_closing_price(int month, int day, int year, const char* symbol) {
//    int ret;
//    unsigned char buf[SSL_MAX_CONTENT_LEN]={0};
//    char query[128] = {0};
//    char result[128] = {0};
//
//    ret = construct_query(month, day, year, symbol, query, sizeof query);
//    if (ret < 0) {
//        return -1;
//    }
//
//    ret = get_page_on_ssl("ichart.yahoo.com", query, NULL, 0, buf, sizeof buf);
//    if (ret != 0){
//        LL_CRITICAL("get_page_on_ssl returned %d\n", ret);
//        return ret;
//    }
//
//    ret = parse_response(buf, result);
//    if (ret < 0)
//        return -1;
//
//    return std::strtod(result, NULL);
    return 0;
}
