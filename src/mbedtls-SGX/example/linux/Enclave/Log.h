#ifndef ENC_LOG_H
#define ENC_LOG_H
#ifdef	__cplusplus
extern "C" {
#endif

#pragma warning (disable: 4273)
extern int printf_sgx(const char *fmt, ...);
#pragma warning (default: 4273) 

enum {
	LOG_LVL_NONE, // 0
	LOG_LVL_CRITICAL, // 1
	LOG_LVL_WARNING, // 2
	LOG_LVL_NOTICE, // 3
	LOG_LVL_LOG, // 4
	LOG_LVL_DEBUG, // 5
	LOG_LVL_NEVER // 6
};
 
#ifndef LOG_BUILD_LEVEL
#ifdef NDEBUG
#define LOG_BUILD_LEVEL LOG_LVL_CRITICAL
#else
#define LOG_BUILD_LEVEL LOG_LVL_DEBUG
#endif
#endif
 
extern unsigned char log_run_level;
 
extern const char * log_level_strings [];
 
// The BUILD_LOG_LEVEL defines what will be compiled in the executable, in production
// it should be set to LVL_NOTICE
 
#define LOG_SHOULD_I( level ) ( level <= LOG_BUILD_LEVEL && level <= log_run_level )

#if (defined(WIN32) || defined(_WIN32))
#define LOG(level, fmt, ...) do {	\
    __pragma(warning (push)) \
    __pragma(warning (disable: 4127)) \
	if ( LOG_SHOULD_I(level) ) { \
		printf_sgx("[%s] %s:%d: " fmt "\n", log_level_strings[level], __FILE__,__LINE__, __VA_ARGS__); \
	} \
} while(0) \
    __pragma(warning (pop))
#else
#define LOG(level, fmt, arg...) do {	\
	if ( LOG_SHOULD_I(level) ) { \
		printf_sgx("[%s] %s:%d: " fmt "\n", log_level_strings[level], __FUNCTION__,__LINE__, ##arg); \
	} \
} while(0)
#endif

#if (defined(WIN32) || defined(_WIN32))
#define LL_DEBUG( fmt, ... ) LOG( LOG_LVL_DEBUG, fmt, __VA_ARGS__ )
#define LL_LOG( fmt, ... ) LOG( LOG_LVL_LOG, fmt,__VA_ARGS__ )
#define LL_NOTICE( fmt,... ) LOG( LOG_LVL_NOTICE, fmt, __VA_ARGS__ )
#define LL_WARNING( fmt, ... ) LOG( LOG_LVL_WARNING, fmt, __VA_ARGS__ )
#define LL_CRITICAL( fmt, ... ) LOG(fmt "\n", __VA_ARGS__ )
#define LL_CRITICAL( fmt, ... ) LOG( LOG_LVL_CRITICAL, fmt, __VA_ARGS__ )
#else
#define LL_DEBUG( fmt, arg... ) LOG( LOG_LVL_DEBUG, fmt, ##arg )
#define LL_LOG( fmt, arg... ) LOG( LOG_LVL_LOG, fmt,##arg )
#define LL_NOTICE( fmt,arg... ) LOG( LOG_LVL_NOTICE, fmt, ##arg )
#define LL_WARNING( fmt, arg... ) LOG( LOG_LVL_WARNING, fmt, ##arg )
#define LL_CRITICAL( fmt, arg... ) LOG( LOG_LVL_CRITICAL, fmt, ##arg )
#endif

#ifdef	__cplusplus
}
#endif

#endif
