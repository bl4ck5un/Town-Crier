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

#include "App/utils.h"

#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>

using std::string;

namespace tc
{
namespace utils
{
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("utils.cpp"));
}
}  // namespace tc

using tc::utils::logger;

/*!
 * \brief   Initialize the enclave:
 *      Step 1: try to retrieve the launch token saved by last transaction
 *      Step 2: call sgx_create_enclave to initialize an enclave instance
 *      Step 3: save the launch token if it is updated
 * \param enclave_name full path to the enclave binary
 * \param eid [out] place to hold enclave id
 */

namespace fs = boost::filesystem;

int initialize_enclave(const char *enclave_name, sgx_enclave_id_t *eid)
{
  if (!fs::exists(enclave_name)) {
    LL_CRITICAL("Enclave file %s doesn't not exist", enclave_name);
    return -1;
  }
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = false;

  /*! Step 1: try to retrieve the launch token saved by last transaction
   *         if there is no token, then create a new one.
   */
  const char *token_path = TOKEN_FILENAME;
  FILE *fp = fopen(token_path, "rb");
  if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
    printf("Warning: Failed to create/open the launch token file \"%s\".\n",
           token_path);
  }

  if (fp != NULL) {
    /* read the token from saved file */
    size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
    if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
      /* if token is invalid, clear the buffer */
      memset(&token, 0x0, sizeof(sgx_launch_token_t));
      printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
    }
  }
  /*! Step 2: call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(
      enclave_name, SGX_DEBUG_FLAG, &token, &updated, eid, NULL);
  if (ret != SGX_SUCCESS) {
    print_error_message(ret);
    if (fp != NULL) fclose(fp);
    return -1;
  }

  /* Step 3: save the launch token if it is updated */
  if (updated == -1 || fp == NULL) {
    /* if the token is not updated, or file handle is invalid, do not perform
     * saving */
    if (fp != NULL) fclose(fp);
    return 0;
  }

  /* reopen the file with write capablity */
  fp = freopen(token_path, "wb", fp);
  if (fp == NULL) return 0;
  size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
  if (write_num != sizeof(sgx_launch_token_t))
    printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
  fclose(fp);
  return 0;
}

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {SGX_ERROR_UNEXPECTED, "Unexpected error occurred.", NULL},
    {SGX_ERROR_INVALID_PARAMETER, "Invalid parameter.", NULL},
    {SGX_ERROR_OUT_OF_MEMORY, "Out of memory.", NULL},
    {SGX_ERROR_ENCLAVE_LOST,
     "Power transition occurred.",
     "Please refer to the sample \"PowerTransition\" for details."},
    {SGX_ERROR_INVALID_ENCLAVE, "Invalid enclave image.", NULL},
    {SGX_ERROR_INVALID_ENCLAVE_ID, "Invalid enclave identification.", NULL},
    {SGX_ERROR_INVALID_SIGNATURE, "Invalid enclave signature.", NULL},
    {SGX_ERROR_OUT_OF_EPC, "Out of EPC memory.", NULL},
    {SGX_ERROR_NO_DEVICE,
     "Invalid SGX device.",
     "Please make sure SGX module is enabled in the BIOS, "
     "and install SGX driver afterwards."},
    {SGX_ERROR_MEMORY_MAP_CONFLICT, "Memory map conflicted.", NULL},
    {SGX_ERROR_INVALID_METADATA, "Invalid enclave metadata.", NULL},
    {SGX_ERROR_DEVICE_BUSY, "SGX device was busy.", NULL},
    {SGX_ERROR_INVALID_VERSION, "Enclave version was invalid.", NULL},
    {SGX_ERROR_INVALID_ATTRIBUTE, "Enclave was not authorized.", NULL},
    {SGX_ERROR_ENCLAVE_FILE_ACCESS, "Can't open enclave file.", NULL},
    {SGX_ERROR_SERVICE_UNAVAILABLE,
     "AE service did not respond or the requested service is not supported.",
     NULL}};

void print_error_message(sgx_status_t ret)
{
  size_t idx = 0;
  size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

  for (idx = 0; idx < ttl; idx++) {
    if (ret == sgx_errlist[idx].err) {
      if (NULL != sgx_errlist[idx].sug)
        printf("Info: %s\n", sgx_errlist[idx].sug);
      printf("Error: %s\n", sgx_errlist[idx].msg);
      break;
    }
  }

  if (idx == ttl) printf("Error: returned %x\n", ret);
}

#include <iomanip>
#include <sstream>

const string sgx_error_message(sgx_status_t ret)
{
  size_t idx = 0;
  size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];
  std::stringstream ss;

  for (idx = 0; idx < ttl; idx++) {
    if (ret == sgx_errlist[idx].err) {
      ss << "Error: " << sgx_errlist[idx].msg;
      if (NULL != sgx_errlist[idx].sug) ss << " " << sgx_errlist[idx].sug;

      return ss.str();
    }
  }

  if (idx == ttl) {
    ss << "ecall returned 0x" << std::hex << ret;
  }

  LL_DEBUG("sgx_error_message: %s", ss.str().c_str());
  return ss.str();
}

#ifdef CONFIG_IMPL_DAEMON
/**
 * \brief This function will daemonize this app
 */
void daemonize(string working_dir, string pid_filename) {}
#endif  // CONFIG_IMPL_DAEMON
