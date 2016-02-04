#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utime.h"

int utime(const char* ds, const char* ts) {
    char year[5], month[3], day[3], hour[3], minute[3];
    int y, mo, d, h, mi, temp;
    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    year[4] = 0;
    month[2] = 0;
    day[2] = 0;
    hour[2] = 0;
    minute[2] = 0;

    memcpy(year, ds, 4);
    memcpy(month, ds+4, 2);
    memcpy(day, ds+6, 2);
    memcpy(hour, ts, 2);
    memcpy(minute, ts+2, 2);
    /*printf("%s %s %s, %s:%s\n", year, month, day, hour, minute);*/
    y = atoi(year);
    mo = atoi(month);
    d = atoi(day);
    h = atoi(hour);
    mi = atoi(minute);

    y = y - 1970;
    temp = y * 365 * 24 * 60 * 60;
    temp += ((y/4) * 24 * 60 * 60);

    mo = mo - 2;
    while (mo >= 0) {
        temp += mdays[mo] * 24 * 60 * 60;
        mo--;
    }

    temp += (d-1) * 24 * 60 * 60;
    temp += h * 60 * 60;
    temp += mi * 60;

    return temp;
}

/*int main(int argc, char* argv[]) {
    const char* ds = "20160122";
    const char* ts = "1045";
    int rc;
    rc = utime(ds, ts);
    printf("Utime: %d\n", rc);

    return 0;
}*/

