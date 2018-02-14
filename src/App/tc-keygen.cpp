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

#define LOGURU_IMPLEMENTATION 1

#include <sgx_error.h>
#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <log4cxx/propertyconfigurator.h>

#include "App/converter.h"
#include "App/Enclave_u.h"
#include "Common/external/base64.hxx"
#include "Common/macros.h"
#include "App/utils.h"

namespace tckeygen {
namespace main {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("tc-keygen.cpp"));
}
}

using std::cout;
using std::endl;
using tckeygen::main::logger;

void print_key(sgx_enclave_id_t eid, string keyfile) {
  LL_INFO("printing key from %s", keyfile.c_str());
  std::ifstream in_keyfile(keyfile);
  if (!in_keyfile.is_open()) {
    std::cerr << "cannot open key file" << endl;
    std::exit(-1);
  }

  std::stringstream buffer;
  buffer << in_keyfile.rdbuf();

  cout << "Sealed Secret: " << buffer.str() << endl;

  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];

  size_t buffer_used = static_cast<size_t>(
      ext::b64_pton(buffer.str().c_str(), secret_sealed, sizeof secret_sealed));

  int ret = 0;
  sgx_status_t ecall_ret;
  ecall_ret = ecdsa_keygen_unseal(
      eid, &ret, reinterpret_cast<sgx_sealed_data_t *>(secret_sealed),
      buffer_used, pubkey, address);
  if (ecall_ret != SGX_SUCCESS || ret != 0) {
    LL_CRITICAL("ecall failed");
    print_error_message(ecall_ret);
    LL_CRITICAL("ecdsa_keygen_unseal returns %d", ret);

    std::exit(-1);
  }
  cout << "PublicKey: " << bufferToHex(pubkey, sizeof pubkey, true) << endl;
  cout << "Address: " << bufferToHex(address, sizeof address, true) << endl;
}

void keygen(sgx_enclave_id_t eid, string keyfile) {
  LL_INFO("generating key to %s", keyfile.c_str());
  unsigned char secret_sealed[SECRETKEY_SEALED_LEN];
  unsigned char pubkey[PUBKEY_LEN];
  unsigned char address[ADDRESS_LEN];

  // call into enclave to fill the above three buffers
  size_t buffer_used = 0;
  int ret;
  sgx_status_t ecall_status;
  ecall_status = ecdsa_keygen_seal(eid, &ret, secret_sealed, &buffer_used,
                                   pubkey, address);
  if (ecall_status != SGX_SUCCESS || ret != 0) {
    LL_CRITICAL("ecall failed");
    print_error_message(ecall_status);
    LL_CRITICAL("ecdsa_keygen_seal returns %d", ret);
    std::exit(-1);
  }

  char secret_sealed_b64[SECRETKEY_SEALED_LEN * 2];
  buffer_used = static_cast<size_t>(
      ext::b64_ntop(secret_sealed, sizeof secret_sealed, secret_sealed_b64,
                    sizeof secret_sealed_b64));

  std::ofstream of(keyfile);
  if (!of.is_open()) {
    LL_CRITICAL("cannot open key file: %s", keyfile.c_str());
    std::exit(-1);
  }
  of.write(secret_sealed_b64, buffer_used);
  of << endl;
  of.close();

  cout << "PublicKey: " << bufferToHex(pubkey, sizeof pubkey, true) << endl;
  cout << "Address: " << bufferToHex(address, sizeof address, true) << endl;
}

namespace po = boost::program_options;

int main(int argc, const char *argv[]) {
  log4cxx::PropertyConfigurator::configure(LOGGING_CONF_FILE);

  string key_input, key_output;
  string enclave_path;

  try {
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "print this message")(
        "enclave,e", po::value(&enclave_path)->required(), "which enclave to use?")(
        "print,p", po::value(&key_input), "print existing keys")(
        "keygen,g", po::value(&key_output), "generate a new key");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cerr << desc << endl;
      return -1;
    }
    po::notify(vm);
  } catch (po::required_option &e) {
    std::cerr << e.what() << endl;
    return -1;
  } catch (std::exception &e) {
    std::cerr << e.what() << endl;
    return -1;
  } catch (...) {
    std::cerr << "Unknown error!" << endl;
    return -1;
  }

  if ((key_input.empty() && key_output.empty()) ||
      (!key_input.empty() && !key_output.empty())) {
    std::cerr << "print specify exactly one command" << endl;
    std::exit(-1);
  }

  sgx_enclave_id_t eid;
  sgx_status_t st;
  int ret;

  ret = initialize_enclave(enclave_path.c_str(), &eid);
  if (ret != 0) {
    LL_CRITICAL("Failed to initialize the enclave");
    std::exit(-1);
  } else {
    LL_INFO("enclave %lu created", eid);
  }

  if (!key_input.empty()) {
    print_key(eid, key_input);
  } else if (!key_output.empty()) {
    keygen(eid, key_output);
  }

  sgx_destroy_enclave(eid);
  LL_INFO("Info: all enclave closed successfully.");
}
