//
// Created by fanz on 4/13/17.
//

#ifndef TOWN_CRIER_CONFIG_H
#define TOWN_CRIER_CONFIG_H

#include <string>

using std::string;

namespace tc {

class Config {
 private:
  const bool DFT_STATUS_ENABLED = false;
  const bool DFT_RUN_AS_DAEMON = false;
  const string DFT_CONFIG_FILE = "config";
  const string DFT_WORKING_DIR = "/tmp/tc";

 public:
  bool is_status_server_enabled() const;
  bool is_run_as_daemon() const;
  const string &get_config_file() const;
  const string &get_working_dir() const;
  const string &get_geth_rpc_addr() const;
  int get_status_server_port() const;
  const string &get_pid_filename() const;
  const string &get_sealed_sig_key() const;
  const string &get_enclave_path() const;
  const string &get_home_dir() const;
  const string &get_current_dir() const;

 private:
  bool opt_status_enabled;
  bool opt_run_as_daemon;
  string opt_config_file;
  string opt_working_dir;

  string cfg_geth_rpc_addr;
  int cfg_status_port;
  string cfg_pid_fn;
  string cfg_sealed_sig_key;
  string cfg_enclave_path;

  string current_dir;
  string home_dir;

 public:
  Config(int argc, const char *argv[]);
  string to_string();
};

}

#endif //TOWN_CRIER_CONFIG_H
