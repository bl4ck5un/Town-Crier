#include "TLSlib_t.h"

#ifndef TSTDIO_H
#define TSTDIO_H

#define _vsnprintf vsnprintf

#if defined(__cplusplus)
extern "C" {
#endif

int printf(const char *fmt, ...);
//char* strcat(char *dest, const char *src);
void itoa(int n, char s[]);
#if defined(__cplusplus)
}
#endif
#endif