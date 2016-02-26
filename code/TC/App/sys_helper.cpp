#include <iostream>
#include <string>
#include <ctime>
#include "Enclave_u.h"
#include "windows.h"
#include "sys_helper.h"
#include <Log.h>

std::string current_datetime()
{
    time_t now = time(0);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return buf;
}

long long rdtsc()
{
    return __rdtsc();
}

void ocall_sleep(int milisec)
{
    LL_NOTICE("Waiting for %d", milisec/1000);
    for (int i = 0; i < milisec / 1000; i++)
    {
        printf(".");
        Sleep(1000);
    }
    printf("\n");
}

time_t ocall_time()
{
    return time(NULL);
}