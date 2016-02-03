#include <cstdio>
#include <cstdlib>

#include "jsonrpc.h"
#include "App.h"
#include "Enclave_u.h"
#include "jsonrpc_httpclient.h"

#include "EthRPC.h"

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

