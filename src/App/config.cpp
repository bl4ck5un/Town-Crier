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

#include "App/config.h"

#include <pwd.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>

using namespace std;

void tc::Config::parseConfigFile() {
  // parse the config files
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::ini_parser::read_ini(configFile, pt);
    enclavePath = pt.get<string>("enclave_path");
    contractAddress = pt.get<string>("tc_address");
    relayRPCAccessPoint = pt.get<int>("RPC.port");
    sealedECDSAKey = pt.get<string>("sealed.sig_key");
    sealedHybridEncryptionkey = pt.get<string>("sealed.hybrid_key");
  } catch (const exception &e) {
    cout << e.what() << endl;
    cout << "please provide with a correct config file" << endl;
    exit(-1);
  }
}

tc::Config::Config(int argc, const char **argv) {
  try {
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "print this message");
    desc.add_options()("measurement,m", po::bool_switch(&isPrintMR)->default_value(false),
                       "print the measurement (MR_ENCLAVE) and exit.");
    desc.add_options()("config,c", po::value(&configFile)->default_value(DFT_CONFIG_FILE),
                       "Path to a config file");
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cerr << desc;
      cerr.flush();
      exit(0);
    }
    po::notify(vm);
  } catch (po::required_option &e) {
    cerr << e.what() << endl;
    exit(-1);
  } catch (exception &e) {
    cerr << e.what() << endl;
    exit(-1);
  } catch (...) {
    cerr << "Unknown error!" << endl;
    exit(-1);
  }

  parseConfigFile();
}

string tc::Config::toString() {
  stringstream ss;
  ss << "Using config file: " << this->getConfigFile() << endl;
  ss << "+ using enclave image: " << this->getEnclavePath() << endl;
  ss << "+ listening for TC relay at port: " << this->getRelayRPCAccessPoint() << endl;
  ss << "+ serving contract at: " << this->getContractAddress();
  return ss.str();
}

const string &tc::Config::getConfigFile() const { return configFile; }
int tc::Config::getRelayRPCAccessPoint() const { return relayRPCAccessPoint; }
const string &tc::Config::getSealedSigKey() const { return sealedECDSAKey; }
const string &tc::Config::getSealedHybridKey() const { return sealedHybridEncryptionkey; }
const string &tc::Config::getEnclavePath() const { return enclavePath; }
const string &tc::Config::getContractAddress() const { return contractAddress; }
bool tc::Config::getIsPrintMR() const { return isPrintMR; }
