//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
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

#include "sgx_utils.h"
#include "sgx_report.h"
#include "string.h"
#include "time.h"
#include "external/keccak.h"
#include "eth_ecdsa.h"
#include "log.h"
#include "Enclave_t.h"
#include "Constants.h"

#include "debug.h"

int ecall_create_report(sgx_target_info_t *quote_enc_info, sgx_report_t *report) {
  sgx_report_data_t data; // user defined data
  int ret = 0;
  memset(&data.d, 0x90, sizeof data.d); // put in some data
  ret = sgx_create_report(quote_enc_info, &data, report);

  hexdump("measurement: ", report->body.mr_enclave.m, SGX_HASH_SIZE);
  return ret;
}

int ecall_get_mr_enclave(unsigned char mr_enclave[32]) {
  sgx_report_t report;

  sgx_status_t ret = sgx_create_report(nullptr, nullptr, &report);
  if (ret != SGX_SUCCESS) {
    LL_CRITICAL("failed to get mr_enclave");
    return -1;
  }

  memcpy(mr_enclave, report.body.mr_enclave.m, SGX_HASH_SIZE);

  return 0;
}

int ecall_time_calibrate(time_t wall_clock, uint8_t wtc_rsv[65]) {
  int ret = 0;
  uint8_t wtc_hash[32];
  ret = keccak((uint8_t *) &wall_clock, sizeof wall_clock, wtc_hash, 32);
  if (ret != 0) {
    LL_CRITICAL("keccak returned %d", ret);
    return ret;
  }
  ret = ecdsa_sign(wtc_hash, sizeof wtc_hash, wtc_rsv, wtc_rsv + 32, wtc_rsv + 64);
  if (ret != 0) {
    LL_CRITICAL("ecdsa_sign() returned %d", ret);
    return ret;
  }
  return ret;
}
