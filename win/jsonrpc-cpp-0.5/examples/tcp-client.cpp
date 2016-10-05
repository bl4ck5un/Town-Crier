/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file tcp-client.cpp
 * \brief Simple JSON-RPC TCP client.
 * \author Sebastien Vincent
 */

#include <cstdio>
#include <cstdlib>

#include "jsonrpc.h"
#include "App.h"

#include "jsonrpc_httpclient.h"

/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */

//  curl -X POST --data '{"jsonrpc":"2.0","method":"web3_clientVersion","params":[],"id":67}' 52.23.173.127:8201

#define SEND_RAW_TX
#define TEST_RAW_TX "f88902850ba43b740083015f9094762f3a1b5502276bd14ecc1cab7e5e8b5cb2719780a460fe47b1000000000000000000000000000000000000000000000000000000000000001b1ca01a51b6b5e8ecf02de07d15714a9bfb9eddb8c2725b2ad1e67d63b71218465e9da07eeba37e51e2f2d280b88950d072337f97217f1904f5e380405970ce023de1ed"
int test_rpc()
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
  query["method"] = "web3_clientVersion";
#ifndef SEND_RAW_TX
  params["from"]        = "0xb7e13de69228c37cdff506ea474f31343af33c05";
  params["to"]          = "0x0dfb71eadd8fe4e3b67fdfc4db2fe12c130996c5";
  params["data"]        = "0x60fe47b1000000000000000000000000000000000000000000000000000000000000029a";
  params["gasPrice"]    = "0xc6cdb129b";
  query["method"] = "eth_sendTransaction";
  query["params"][0] = params;
#else
  params = TEST_RAW_TX;
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
  if(HttpClient.Recv(responseStr) != -1)
  {
    std::cout << "Received: " << responseStr << std::endl;
  }
  else
  {
    std::cerr << "Error while receiving data!" << std::endl;
  }

  HttpClient.Close();
  networking::cleanup();

  return EXIT_SUCCESS;
}

