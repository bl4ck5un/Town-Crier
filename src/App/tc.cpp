#include <jsonrpccpp/server/connectors/httpserver.h>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <atomic>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>

#include "sgx_uae_service.h"
#include "sgx_urts.h"

#include "Constants.h"
#include "Enclave_u.h"
#include "EthRPC.h"
#include "Log.h"
#include "Monitor.h"
#include "StatusRpcServer.h"
#include "attestation.h"
#include "bookkeeping/database.hxx"
#include "request-parser.hxx"
#include "stdint.h"
#include "utils.h"
#include "utils.h"

namespace po = boost::program_options;

extern ethRPCClient *rpc_client;
jsonrpc::HttpClient *httpclient;

std::atomic<bool> quit(false);
void exitGraceful(int) { quit.store(true); }

int main(int argc, const char *argv[]) {
    boost::filesystem::path current_path = boost::filesystem::current_path();

    bool options_rpc = false;
    bool options_daemon = false;
    string options_config = "config";

    try {
        po::options_description desc("Allowed options");
        desc.add_options()(
            "help,h", "print this message")(
            "rpc", po::bool_switch(&options_rpc)->default_value(false), "Launch RPC server")(
            "daemon,d", po::bool_switch(&options_daemon)->default_value(false), "Run TC as a daemon")(
            "config,c", po::value(&options_config)->default_value("config"), "Path to a config file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            cerr << desc << endl;
            return -1;
        }

        po::notify(vm);
    }

    catch (po::required_option &e) {
        cerr << e.what() << endl;
        return -1;
    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return -1;
    } catch (...) {
        cerr << "Unknown error!" << endl;
        return -1;
    }

    //! 
    string working_dir;
    string pid_filename;

    //! parse config files
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::ini_parser::read_ini(options_config, pt);
        string st = pt.get<string>("RPC.RPChost");
        BOOST_LOG_TRIVIAL(info) << "RPC.RPChost: " << st;
        httpclient = new jsonrpc::HttpClient(st);
        rpc_client = new ethRPCClient(*httpclient);

        working_dir = pt.get<string>("daemon.working_dir");
        pid_filename = pt.get<string>("daemon.pid_file");
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        exit(-1);
    }

    BOOST_LOG_TRIVIAL(info) << "config file " << options_config << " loaded";
    BOOST_LOG_TRIVIAL(info) << "CWD: " << working_dir;
    BOOST_LOG_TRIVIAL(info) << "PID file: " << pid_filename;

    int ret;
    sgx_enclave_id_t eid;
    sgx_status_t st;

    //! register Ctrl-C handler
    std::signal(SIGINT, exitGraceful);

    jsonrpc::HttpServer httpServer(8123);
    StatusRpcServer statusRpcServer(httpServer, eid);
    if (options_rpc) {
        BOOST_LOG_TRIVIAL(info) << "Setting up RPC Server";
        statusRpcServer.StartListening();
    }

    if (options_daemon) {
        boost::filesystem::create_directory(boost::filesystem::path(working_dir));
        daemonize(working_dir, pid_filename);
    }
    const static string db_name = working_dir + "TC.db";
    bool create_db = false;
    if (boost::filesystem::exists(db_name) && !options_daemon) {
        std::cout << "Do you want to clean up the database? y/[n] ";
        std::string new_db;
        std::getline(std::cin, new_db);
        create_db = new_db == "y";
    } else {
        create_db = true;
    }
    OdbDriver driver("TC.db", create_db);

    int nonce_offset = 0;

    ret = initialize_tc_enclave(&eid);
    if (ret != 0) {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize the enclave";
        std::exit(-1);
    } else {
        BOOST_LOG_TRIVIAL(info) << "enclave " << eid << " created";
    }

    Monitor monitor(driver, eid, nonce_offset, quit);
    monitor.loop();

    if (options_rpc) {
        statusRpcServer.StopListening();
    }
    sgx_destroy_enclave(eid);
    delete rpc_client;
    delete httpclient;
    BOOST_LOG_TRIVIAL(info) << "all enclave closed successfully";
}
