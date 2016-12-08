#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <string>

#include "App.h"
#include "Enclave_u.h"
#include "Log.h"
#include "EthRPC.h"


#include "Converter.h"
#include "Constants.h"

#include "Utils.h"

#include <sstream>

using namespace jsonrpc;

ethRPCClient *c;

int send_transaction(std::string hostname, unsigned port, char* raw)
{
    ( void )(hostname);
    ( void )(port);
    std::string res;
    std::string param1(raw);
    try{
        if (!c) return -1;
        res = c->eth_sendRawTransaction(param1);

        LL_CRITICAL("Response recorded in the blockchain.");
        LL_CRITICAL("TX: %s", res.c_str());

        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("Error in send_transaction: %s", e.what());
        return -1;
    }
}

#if 0
int eth_new_filter(std::string hostname, unsigned port, std::string& id, int from, int to)
{
    /*
    > filter_opt
    {
        address: "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b",
        fromBlock: from,
        toBlock: to,
        topics: ["0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0"]
    }

    > web3.sha3("RequestInfo(uint64,uint8,address,uint256,address,bytes32,bytes32[])")
    "8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0"

    * note that uint -> uint256 before applying SHA3
    * see: https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    */
    Json::Value query;
    Json::Value filter_opt;
    Json::Value result;

    std::stringstream from_s, to_s;
    from_s << "0x" << std::hex << from;
    to_s << "0x" << std::hex << to;

    filter_opt["address"] = "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b";
    filter_opt["topics"][0] = "0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0";
    filter_opt["fromBlock"] = from_s.str();
    filter_opt["toBlock"] = to_s.str();

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_newFilter";
    query["params"][0] = filter_opt;

    try
    {
        rpc_base(hostname, port, query, result);
    }
    catch (std::exception& re)
    {
        LL_CRITICAL("%s", re.what());
        std::cout << result << std::endl;
        return -1;
    }
    id = result.asString();
    return EXIT_SUCCESS;
}
#endif

int eth_new_filter(std::string hostname, unsigned port, std::string& id, int from, int to)
{
    Json::Value filter_opt;
    try {
        filter_opt["address"] = TC_ADDRESS;
        filter_opt["topics"][0] = "0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0";
        filter_opt["fromBlock"] = from;
        filter_opt["toBlock"] = to;

        if (!c) return -1;
        id = c->eth_newFilter(filter_opt);
        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("%s", e.what());
        return -1;
    }
}

#if 0
int eth_getfilterlogs(std::string hostname, unsigned port, std::string filter_id, Json::Value& result)
{
    Json::Value query;
    Json::FastWriter writer;
    // int to hex string

    std::stringstream filter_id_s;
    filter_id_s << "0x" << std::hex << filter_id;

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_getFilterLogs";
    query["params"][0] = filter_id;

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
#endif

int eth_getfilterlogs(std::string hostname, unsigned port, std::string filter_id, Json::Value& result) {
    try {
        if (!c) return -1;
        result = c->eth_getFilterLogs(filter_id);
        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("%s", e.what());
        return -1;
    }
}

#if 0
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
#endif

long eth_blockNumber(std::string hostname, unsigned port)
{
    try {
        unsigned long ret;
        if (!c) return -1;
        std::string blk = c->eth_blockNumber();
        std::stringstream ss;
        ss << std::hex << blk;
        ss >> ret;
        return ret;
    }
    catch (JsonRpcException e){
        LL_CRITICAL("eth_blockNumber: %s", e.what());
        return -1;
    }
}

Request::Request(uint8_t *input) {
    this->id = u64_from_b(input + 0x20 - sizeof(this->id));
    this->type = u64_from_b(input + 0x40 - sizeof(this->type));

    memcpy(this->requester, input + 0x40 + 16, 20);

    this->fee = u32_from_b(input + 0x80 - sizeof(this->fee));
    memcpy(this->callback, input + 0x80 + 16, 20);

    memcpy(this->param_hash, input + 0xa0, 32);

    this->data_len = u32_from_b(input + 0x100 - sizeof(this->data_len)) * 32;
    this->data = (uint8_t*) malloc(this->data_len);

    if (this->data == NULL) {
        LL_CRITICAL("Failed to allocate memory. RID=%ld", this->id);
        throw std::runtime_error("failed to malloc");
    }
    else {
        memcpy(this->data, input + 0x100, this->data_len);
    }
}

Request::~Request() {
    if (this->data != NULL) {
        free(this->data);
    }
}
