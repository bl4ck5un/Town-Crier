//
// Created by fanz on 4/12/17.
//

#ifndef TOWN_CRIER_SCRAPER_UTILS_H
#define TOWN_CRIER_SCRAPER_UTILS_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

string url_encode(const char *str);
string url_decode(const char *str);

#endif //TOWN_CRIER_SCRAPER_UTILS_H
