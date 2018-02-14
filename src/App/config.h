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

#ifndef SRC_APP_CONFIG_H_
#define SRC_APP_CONFIG_H_

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <string>

using std::string;

namespace po = boost::program_options;
namespace fs = boost::filesystem;


namespace tc {

class Config {
 private:
  const string DFT_CONFIG_FILE = "/tc/conf/config-sim-test";
  const string DFT_WORKING_DIR = "/tc";
  po::variables_map vm;

 public:
  bool isStatusServerEnabled() const;
  const string &getConfigFile() const;
  const string &getWorkingDir() const;
  const string &getGethRpcAddr() const;
  int get_status_server_port() const;
  const string &getPidFilename() const;
  const string &getSealedSigKey() const;
  const string &getSealedHybridKey() const;
  const string &getEnclavePath() const;
  const string &getHomeDir() const;
  const string &getCurrentDir() const;
  bool printMR() const;
  const po::variables_map& getOpts() const;

 private:
  bool opt_mrenclave;
  string opt_config_file;
  string opt_working_dir;

  string cfg_geth_rpc_addr;
  bool cfg_status_rpc_enabled;
  int cfg_status_port;
  string cfg_pid_fn;
  string cfg_sealed_sig_key;
  string cfg_sealed_hybrid_key;
  string cfg_enclave_path;

  string current_dir;
  string home_dir;

  void parseConfigFile();

 public:
  Config(int argc, const char *argv[]);
  Config(const po::options_description &, int argc, const char *argv[]);
  string toString();
};

}  // namespace tc

#endif  // SRC_APP_CONFIG_H_
