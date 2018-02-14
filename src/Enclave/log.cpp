#include "log.h"

unsigned char log_run_level = LOG_LVL_DEBUG;
// unsigned char log_run_level = LOG_LVL_NOTICE;
const char *log_level_strings[] = {
    "NONE", // 0
    "CRIT", // 1
    "WARN", // 2
    "NOTI", // 3
    " LOG", // 4
    "DEBG" // 5
};