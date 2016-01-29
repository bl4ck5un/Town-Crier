#include "TLSlib_t.h"

#ifndef TSTDIO_H
#define TSTDIO_H

#define _vsnprintf vsnprintf

int printf(const char *fmt, ...);

char* strcat(char *dest, const char *src);

void itoa(int n, char s[]);

#endif