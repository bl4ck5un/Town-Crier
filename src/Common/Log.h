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
 * the TownCrier source code. No other rights to use TownCrier and its
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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef ENCLAVE_STD_ALT
// printf
#include "glue.h"
// for ocall_log_FATAL etc
#include "Enclave_t.h"
#else
#include "loguru.hpp"
#endif


#ifndef ENC_LOG_H
#define ENC_LOG_H

#ifdef    __cplusplus
#define _FALSE false
#else
#define _FALSE 0
#endif

#ifdef    __cplusplus
extern "C" {
#endif

#ifdef ENCLAVE_STD_ALT
// macros for use in the enclave
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(level,fmt,arg... ) \
do { \
  char __log_buf[BUFSIZ] = {'\0'};\
  snprintf(__log_buf, BUFSIZ, "[%s:%d] " fmt, __FILENAME__, __LINE__, ##arg);\
  ocall_log_##level(__log_buf);\
} while(_FALSE)
#else
#define LOG(level,fmt,arg... ) \
do { \
  LOG_F(level,fmt,##arg ); \
} while(_FALSE)
#endif

#define LOG_LEVEL_TRACE 3
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_LOG 1
#define LOG_LEVEL_INFO 0

#define LL_TRACE( fmt, arg... )     LOG(3, fmt, ##arg )
#define LL_DEBUG( fmt, arg... )     LOG(2, fmt, ##arg )
#define LL_LOG( fmt, arg... )       LOG(1, fmt, ##arg )
#define LL_INFO( fmt, arg... )      LOG(INFO,    fmt, ##arg )
#define LL_WARNING( fmt, arg... )   LOG(WARNING, fmt, ##arg )
#define LL_ERROR( fmt, arg... )     LOG(ERROR, fmt, ##arg )
#define LL_CRITICAL( fmt, arg... )  LOG(ERROR, fmt, ##arg )

#ifdef __cplusplus
}
#endif

#endif
