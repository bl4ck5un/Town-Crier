//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include <sgx_tseal.h>
#include <unistd.h>
#include <string.h>

#include <ctime>
#include <iostream>
#include <string>

#include "Common/Log.h"
#include "Enclave_u.h"

int ocall_print_string(const char *str) {
  /* Proxy/Bridge will check the length and null-terminate
   * the input string to prevent buffer overflow.
   */
  int ret = printf("%s", str);
  fflush(stdout);
  return ret;
}

/* defining log functions for enclave's usage */

#define __OCALL_LOG_FUNC(LVL) \
  void ocall_log_##LVL(const char* str) { LOG_F(LVL, "%s", str); }

__OCALL_LOG_FUNC(FATAL)
__OCALL_LOG_FUNC(ERROR)
__OCALL_LOG_FUNC(WARNING)
__OCALL_LOG_FUNC(INFO)
__OCALL_LOG_FUNC(1)
__OCALL_LOG_FUNC(2)
__OCALL_LOG_FUNC(3)
__OCALL_LOG_FUNC(4)
__OCALL_LOG_FUNC(5)
__OCALL_LOG_FUNC(6)
__OCALL_LOG_FUNC(7)
__OCALL_LOG_FUNC(8)
__OCALL_LOG_FUNC(9)

void ocall_sleep(int milisec) {
  LL_INFO("Waiting for %d", milisec / 1000);
  for (int i = 0; i < milisec / 1000; i++) {
    printf(".");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  }
  printf("\n");
}

time_t ocall_time() { return time(NULL); }

void write(uint32_t sealed_data_size, sgx_sealed_data_t *p_sealed_data,
           char *filename) {
  FILE *fp = fopen(filename, "wb");
  fwrite(p_sealed_data, sealed_data_size, 1, fp);
}

int ocall_log_lvl() {
  return loguru::g_stderr_verbosity;
}

int ocall_is_debug() {
  return loguru::g_stderr_verbosity >= LOG_LEVEL_DEBUG;
}
