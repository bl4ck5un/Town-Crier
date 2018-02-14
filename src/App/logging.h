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
#include <log4cxx/logger.h>


#ifndef APP_LOG_H
#define APP_LOG_H

constexpr auto LOGGING_CONF_FILE = "/tc/conf/logging.conf";

// a universal false for C++/C
#ifdef    __cplusplus
#define _FALSE false
#else
#define _FALSE 0
#endif

/***************************************************/
/******* Logging utilities for untrusted world *****/
/***************************************************/

extern char log_buffer[BUFSIZ];

#define LOG_TO_LOG4CXX(FUNC, fmt, arg...) \
  do { \
    snprintf(log_buffer, BUFSIZ, fmt, ##arg); \
    FUNC(logger, log_buffer) } \
  while (_FALSE);

#define LL_LOG(fmt, arg...)       LOG_TO_LOG4CXX(LOG4CXX_TRACE, fmt, ##arg)
#define LL_DEBUG(fmt, arg...)     LOG_TO_LOG4CXX(LOG4CXX_DEBUG, fmt, ##arg)
#define LL_TRACE(fmt, arg...)     LOG_TO_LOG4CXX(LOG4CXX_TRACE, fmt, ##arg)
#define LL_INFO(fmt, arg...)      LOG_TO_LOG4CXX(LOG4CXX_INFO, fmt, ##arg)
#define LL_WARNING(fmt, arg...)   LOG_TO_LOG4CXX(LOG4CXX_WARN, fmt, ##arg)
#define LL_ERROR(fmt, arg...)     LOG_TO_LOG4CXX(LOG4CXX_ERROR, fmt, ##arg)
#define LL_CRITICAL(fmt, arg...)  LOG_TO_LOG4CXX(LOG4CXX_FATAL, fmt, ##arg)

#endif // ENC_LOG_H