//
// Created by fanz on 4/13/17.
//

#include "config.h"

#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <pwd.h>

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

inline const char* homedir() {
  const char* home_dir;
  if ((home_dir = getenv("HOME")) == NULL) {
    home_dir = getpwuid(getuid())->pw_dir;
  }

  return home_dir;
}

tc::Config::Config(int argc, const char **argv) {
  this->current_dir = fs::current_path().string();
  this->home_dir = homedir();

  try {
    po::options_description desc("Allowed options");
    desc.add_options()(
        "help,h", "print this message")(
        "rpc", po::bool_switch(&opt_status_enabled)->default_value(DFT_STATUS_ENABLED), "Launch RPC server")(
        "daemon,d", po::bool_switch(&opt_run_as_daemon)->default_value(DFT_RUN_AS_DAEMON), "Run TC as a daemon")(
        "config,c", po::value(&opt_config_file)->default_value(DFT_CONFIG_FILE), "Path to a config file")(
        "cwd", po::value(&opt_working_dir)->default_value(DFT_WORKING_DIR), "Working directory (where log and db are stored");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cerr << desc << endl;
      exit(0);
    }
    po::notify(vm);
  }
  catch (po::required_option &e) {
    cerr << e.what() << endl;
    exit(-1);
  } catch (std::exception &e) {
    cerr << e.what() << endl;
    exit(-1);
  } catch (...) {
    cerr << "Unknown error!" << endl;
    exit(-1);
  }

  // parse the config files
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::ini_parser::read_ini(opt_config_file, pt);
    cfg_geth_rpc_addr = pt.get<string>("RPC.RPChost");
//    httpclient = new jsonrpc::HttpClient(st);
//    rpc_client = new ethRPCClient(*httpclient);

    cfg_pid_fn = pt.get<string>("daemon.pid_file");
    cfg_status_port = pt.get<int>("status.port");
    cfg_sealed_sig_key = pt.get<string>("sealed.sig_key");
    cfg_enclave_path = pt.get<string>("init.enclave_path");

  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    exit(-1);
  }

  cout << "config done." << endl;
}

string tc::Config::to_string() {
  stringstream ss;
  ss << "status server enabled: " << opt_status_enabled << endl;
  ss << "status server port: " << cfg_status_port << endl;
  ss << "run as daemon: " << opt_run_as_daemon << endl;
  ss << "using config file: " << opt_config_file << endl;
  ss << "working dir set to: " << opt_working_dir << endl;
  ss << "geth rpc addr: " << cfg_geth_rpc_addr << endl;
  ss << "pid filename: " << cfg_pid_fn << endl;
  ss << "enclave image used: " << cfg_enclave_path << endl;

  return ss.str();
}

bool tc::Config::is_status_server_enabled() const {
  return opt_status_enabled;
}
bool tc::Config::is_run_as_daemon() const {
  return opt_run_as_daemon;
}
const string &tc::Config::get_config_file() const {
  return opt_config_file;
}
const string &tc::Config::get_working_dir() const {
  return opt_working_dir;
}
const string &tc::Config::get_geth_rpc_addr() const {
  return cfg_geth_rpc_addr;
}
int tc::Config::get_status_server_port() const {
  return cfg_status_port;
}
const string &tc::Config::get_pid_filename() const {
  return cfg_pid_fn;
}
const string &tc::Config::get_sealed_sig_key() const {
  return cfg_sealed_sig_key;
}
const string &tc::Config::get_enclave_path() const {
  return cfg_enclave_path;
}
const string& tc::Config::get_current_dir() const {
  return current_dir;
}
const string& tc::Config::get_home_dir() const {
  return home_dir;
}