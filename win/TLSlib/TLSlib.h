#pragma once

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

#if defined(_MSC_VER)
/* The following macros are MSVC only */

# define MKOCALL_INT(fname, ...) int ret; \
	ocall_ ## fname ## (&ret, ## __VA_ARGS__ ## ); \
	return ret;\

# define MKOCALL_VOID(fname, ...) ocall_ ## fname ## (## __VA_ARGS__ ## );

#elif defined(__GNUC__)
/* The following macros are GCC only */


#endif /* __GNUC__ */