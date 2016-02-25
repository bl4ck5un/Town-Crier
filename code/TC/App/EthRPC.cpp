#include <cstdio>
#include <cstdlib>

#include "jsonrpc.h"
#include "App.h"
#include "Enclave_u.h"
#include "jsonrpc_httpclient.h"
#include "Log.h"

#include "EthRPC.h"

#include <iomanip>
#include <sstream>


static int rpc_base(std::string hostname, unsigned port, Json::Value& query, Json::Value& response)
{
    Json::Rpc::HttpClient HttpClient(hostname, port);
    Json::FastWriter writer;
    std::string queryStr;
    std::string responseStr;
    Json::Value res;
    Json::Reader resReader;
    std::stringstream str;

    if(!networking::init())
    {
        throw std::runtime_error("Networking initialization failed");
    }

    queryStr = writer.write(query);
    LL_LOG("query: %s", queryStr.c_str());

    if(HttpClient.Send(queryStr) != 0)
    {
        throw std::runtime_error("Error while sending data!");
    }

    if(HttpClient.Recv(responseStr) == -1)
    {
        throw std::runtime_error("Error while receiving data!");
    }

    if (!resReader.parse(responseStr, res))
    {
        std::cout << "Can't parse" << responseStr << std::endl;
        throw std::runtime_error("Parse Error");
    }

    if (!res["error"].isNull())
    {
        throw std::invalid_argument(res["error"]["message"].asString());
    }

    response = res["result"];
    
    LL_LOG("response: %s", writer.write(response).c_str());

    HttpClient.Close();
    networking::cleanup();

    return EXIT_SUCCESS;
}


/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */

//  curl -X POST --data '{"jsonrpc":"2.0","method":"web3_clientVersion","params":[],"id":67}' 52.23.173.127:8201

#define SEND_RAW_TX
//#undef SEND_RAW_TX

int send_transaction(char* raw)
{
  Json::Rpc::HttpClient HttpClient(std::string("localhost"), 8200);
  Json::FastWriter writer;
  std::string queryStr;
  std::string responseStr;
  Json::Value query;
  Json::Value params;
  
  if(!networking::init())
  {
    std::cerr << "Networking initialization failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  query["jsonrpc"] = "2.0";
  query["id"] = 1;
#ifndef SEND_RAW_TX
  params["from"]        = "0xb7e13de69228c37cdff506ea474f31343af33c05";
  params["to"]          = "0x0dfb71eadd8fe4e3b67fdfc4db2fe12c130996c5";
  params["data"]        = "0x60fe47b1000000000000000000000000000000000000000000000000000000000000029a";
  params["gasPrice"]    = "0xc6cdb129b";
  query["method"] = "eth_sendTransaction";
  query["params"][0] = params;
#else
  params = raw;
  query["method"] = "eth_sendRawTransaction";
  query["params"][0] = params;
#endif
  queryStr = writer.write(query);
  std::cout << "Query is: " << queryStr << std::endl;

  if(HttpClient.Send(queryStr) == -1)
  {
    std::cerr << "Error while sending data!" << std::endl;
    exit(EXIT_FAILURE);
  }

    /* wait the response */
    Json::Value res;
    Json::Reader resReader;
    if(HttpClient.Recv(responseStr) != -1)
    {
        if (!resReader.parse(responseStr, res))
        {
            printf("Parse Error\n");
            return -1;
        }
        Json::Value error = res["error"];
        if (!error.isNull())
        {
            std::cout << "ERROR: " << error["message"] << std::endl;
            return -1;
        }
        else
        {
            Json::Value result = res["result"];
            std::cout << "Result: " << result << std::endl;
        }
    }
    else
    {
    std::cerr << "Error while receiving data!" << std::endl;
    }

  HttpClient.Close();
  networking::cleanup();

  return EXIT_SUCCESS;
}

int eth_new_filter(std::string hostname, unsigned port, int* id, int from, int to)
{
    /*
    > filter_opt
{
  address: "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b",
  fromBlock: 1,
  toBlock: "latest",
  topics: []
}
    */
    Json::Value query;
    Json::Value filter_opt;
    Json::Value result;

    std::stringstream from_s, to_s;
    from_s << "0x" << std::hex << from;
    to_s << "0x" << std::hex << to;

    filter_opt["address"] = "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b";
//    filter_opt["topics"] = Json::arrayValue;
    filter_opt["topics"][0] = "0x28dc24a2a8e41b6447d3e50c69bdf1155bbdc688627f7b29ac10ed2877b4cd33";
    filter_opt["fromBlock"] = from_s.str();
    filter_opt["toBlock"] = to_s.str();

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_newFilter";
    query["params"][0] = filter_opt;

    rpc_base(hostname, port, query, result);

    std::cout << result << std::endl;
    *id = std::strtol(result.asCString(), NULL, 16);
    return EXIT_SUCCESS;
}

int eth_getfilterlogs(std::string hostname, unsigned port, long filter_id, Json::Value& result)
{
    Json::Value query;
    Json::FastWriter writer;
    // int to hex string

    std::stringstream filter_id_s;
    filter_id_s << "0x" << std::hex << filter_id;

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_getFilterLogs";
    query["params"][0] = filter_id_s.str();

    try
    {
        rpc_base(hostname, port, query, result);
    }
    catch (std::exception& re)
    {
        LL_CRITICAL("%s", re.what());
        return -1;
    }
    return EXIT_SUCCESS;
}

unsigned long eth_blockNumber(std::string hostname, unsigned port)
{
    Json::Value query;
    Json::FastWriter writer;

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_blockNumber";

    LL_DEBUG("method: %s", query["method"].asString().c_str());

    Json::Value resp;
    rpc_base(hostname, port, query, resp);

    if (resp.isString())
    {
        std::string blockNumber = resp.asString();
        return std::strtoul(blockNumber.c_str(), NULL, 16);
    }
        
    return -1;
}