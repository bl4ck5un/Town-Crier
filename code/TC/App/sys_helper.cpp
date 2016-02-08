#include <iostream>
#include <string>
#include <ctime>

#include "sys_helper.h"

std::string current_datetime()
{
    time_t now = time(0);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return buf;
}