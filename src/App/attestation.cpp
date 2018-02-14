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

#include <sgx_report.h>
#include <sgx_uae_service.h>
#include <sgx_utils.h>
#include <time.h>
#include <string>

#include "App/Enclave_u.h"
#include "App/attestation.h"
#include "App/converter.h"
#include "App/tc_exception.h"
#include "App/utils.h"
#include "Common/Constants.h"
#include "logging.h"

using std::vector;
using std::to_string;
using std::invalid_argument;

namespace tc {
namespace attestation {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("attestation.cpp"));
}
}

using tc::attestation::logger;

int time_calibrate(sgx_enclave_id_t eid) {
  time_t wtc_time = time(NULL);
  uint8_t time_sig[65];
  int ret = 0;
  sgx_status_t st;

  st = ecall_time_calibrate(eid, &ret, wtc_time, time_sig);
  return ret;
}

void get_attestation(sgx_enclave_id_t eid, vector<uint8_t> *out) {
  if (out == nullptr) {
    throw invalid_argument("null output ptr");
  }
  sgx_target_info_t qe_info;
  sgx_epid_group_id_t p_gid;
  sgx_report_t report;
  sgx_spid_t spid;
  int ret;
  sgx_status_t ecall_ret;

  sgx_init_quote(&qe_info, &p_gid);
  memset(qe_info.reserved1, 0, sizeof qe_info.reserved1);
  memset(qe_info.reserved2, 0, sizeof qe_info.reserved2);
  ecall_ret = ecall_create_report(eid, &ret, &qe_info, &report);
  if (ecall_ret != SGX_SUCCESS || ret) {
    LL_DEBUG("ecall_create_report: ecall_ret=%x, ret=%x", ecall_ret, ret);
    throw tc::EcallException(ecall_ret,
                             "ecall_create_report returned " + to_string(ret));
  }

  uint8_t spid_tc[16] = {
      0x03, 0xD4, 0x81, 0x28,
      0x36, 0x6F, 0x1C, 0xD7,
      0x4F, 0xCA, 0x49, 0x0D,
      0x9B, 0x85, 0xB6, 0xAB,
  };

  memcpy(spid.id, spid_tc, sizeof spid_tc);

  uint32_t quote_size;
  sgx_calc_quote_size(nullptr, 0, &quote_size);
  auto *quote = reinterpret_cast<sgx_quote_t *>(malloc(quote_size));

  ecall_ret = sgx_get_quote(&report,
                            SGX_LINKABLE_SIGNATURE,
                            &spid, nullptr, nullptr,
                            0, nullptr, quote, quote_size);
  if (ecall_ret != SGX_SUCCESS) {
    print_error_message((sgx_status_t)ret);
    throw tc::EcallException(ecall_ret, "sgx_get_quote failed");
  }

  out->insert(out->begin(), reinterpret_cast<uint8_t *>(quote),
              reinterpret_cast<uint8_t *>(quote) + quote_size);
  free(quote);
}

string get_mr_enclave(sgx_enclave_id_t eid) {
  int ret;
  unsigned char mr_enclave[SGX_HASH_SIZE];
  sgx_status_t ecall_ret = ecall_get_mr_enclave(eid, &ret, mr_enclave);
  if (ecall_ret != SGX_SUCCESS) {
    print_error_message((sgx_status_t)ret);
    throw tc::EcallException(ecall_ret, "get_mr_enclave failed");
  }
  return bufferToHex(mr_enclave, SGX_HASH_SIZE);
}
