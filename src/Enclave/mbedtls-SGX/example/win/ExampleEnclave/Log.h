/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially
 * docketed at the Center for Technology Licensing at Cornell University
 * as D-7364, developed through research conducted at Cornell University,
 * and its associated copyrights solely for educational, research and
 * non-profit purposes without fee is hereby granted, provided that the
 * user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling
 * the TowCrier source code. No other rights to use TownCrier and its
 * associated copyrights for any other purpose are granted herein,
 * whether commercial or non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial
 * products or use TownCrier and its associated copyrights for commercial
 * purposes must contact the Center for Technology Licensing at Cornell
 * University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
 * ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
 * commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
 * ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
 * UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
 * REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
 * OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
 * OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
 * PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science
 * Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
 * CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
 * Google Faculty Research Awards, and a VMWare Research Award.
 */

#ifndef ENC_LOG_H
#define ENC_LOG_H
#ifdef	__cplusplus
extern "C" {
#endif

#pragma warning (disable: 4273)
extern int printf(const char *fmt, ...);
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
		printf("[%s] %s:%d: " fmt "\n", log_level_strings[level], __FILE__,__LINE__, __VA_ARGS__); \
	} \
} while(0) \
    __pragma(warning (pop))
#else
#define LOG(level, fmt, arg...) do {	\
	if ( LOG_SHOULD_I(level) ) { \
		printf("[%s] %s:%d: " fmt "\n", log_level_strings[level], __FUNCTION__,__LINE__, ##arg); \
	} \
} while(0)
#endif

#if (defined(WIN32) || defined(_WIN32))
#define LL_DEBUG( fmt, ... ) LOG( LOG_LVL_DEBUG, fmt, __VA_ARGS__ )
#define LL_LOG( fmt, ... ) LOG( LOG_LVL_LOG, fmt,__VA_ARGS__ )
#define LL_NOTICE( fmt,... ) LOG( LOG_LVL_NOTICE, fmt, __VA_ARGS__ )
#define LL_WARNING( fmt, ... ) LOG( LOG_LVL_WARNING, fmt, __VA_ARGS__ )
//#define LL_CRITICAL( fmt, ... ) printf(fmt "\n", __VA_ARGS__ )
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