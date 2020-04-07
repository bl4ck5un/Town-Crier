#include "logging.h"

#include <log4cxx/logger.h>

char log_buffer[BUFSIZ] = {'\0'};

#define OCALL_LOGGING_ADAPTOR(FUNC)                           \
  {                                                           \
    do {                                                      \
      FUNC(logger, "(" << file << ":" << line << ") " << msg) \
    } while (_FALSE);                                         \
  }

/*!
 *
 * @param level: to be consistent with
 *
 * LOG_LVL_CRITICAL,    // 0
 * LOG_LVL_WARNING,     // 1
 * LOG_LVL_NOTICE,      // 2
 * LOG_LVL_DEBUG,       // 3
 *
 * @param file
 * @param line
 * @param msg
 */
void ocall_logging(int level, const char* file, int line, const char* msg)
{
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("Enclave"));
  switch (level) {
    case 0:
      OCALL_LOGGING_ADAPTOR(LOG4CXX_ERROR)
      break;
    case 1:
      OCALL_LOGGING_ADAPTOR(LOG4CXX_WARN)
      break;
    case 2:
      OCALL_LOGGING_ADAPTOR(LOG4CXX_INFO)
      break;
    case 3:
      OCALL_LOGGING_ADAPTOR(LOG4CXX_DEBUG)
      break;
    default:
      return;
  }
}
