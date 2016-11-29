/* EthRPC is a wrapper function around etherium Blockchain calls
 * It provides functionality for talking to the blockchain and checking specific state
 * Additionally, the Request object parses the data recieved from the blockchain
 */

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <string>
#include <sstream>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "App.h"
#include "Enclave_u.h"
#include "Log.h"
#include "EthRPC.h"
#include "Converter.h"
#include "Constants.h"
#include "ethrpcclient.h"
#include "Utils.h"


using namespace jsonrpc;

HttpClient httpclient("http://localhost:8200");
ethRPCClient c(httpclient);						/* The Etherium HTTP client */ 

/* send_transaction [raw] sends data [raw] to remote host httpclient
 * listenign in on port [port]
 * Note: [hostname] and [port] are unsed parameters for now
 */ 
int send_transaction(char* raw)
{
    if(raw == NULL){ 
    	return -1;
    }
    //Casting used to suppress compiler warnings
    std::string res;
    std::string param1(raw);

	//Send [raw] to [c]
    res = c.eth_sendRawTransaction(param1);

    LL_CRITICAL("Response recorded in the blockchain.");
    LL_CRITICAL("TX: %s", res.c_str());

    return 0;

}

/* Returns a filter from block [from] to block [to], writes the value of the filter into [id]
 * Postcondition: [id] is a valid id that can be used with eth_get_filter_logs
 */
int eth_new_filter(std::string& id, int from, int to)
{
    if(&id == NULL || from < 0 || to < 0){
    	return -1;
    }

    Json::Value filter_opt;
    filter_opt["address"] = TC_ADDRESS;
    filter_opt["topics"][0] = "0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0";
    filter_opt["fromBlock"] = from;
    filter_opt["toBlock"] = to;

    id = c.eth_newFilter(filter_opt);
    return 0;

}


/* eth_getfilterlogs [hostname] [port] [filter_id] [result] returns the logged events of [filter_id]
 * Given the [filter_id] writes to [result] an array containing the required data
 */
int eth_getfilterlogs(std::string filter_id, Json::Value& result) {
    if(&filter_id == NULL || &result == NULL){
    	return -1;
    }
    result = c.eth_getFilterLogs(filter_id);
    return 0;
}

/* eth_blockNumber [hostname] [port] returns the block number of C
 * Postcondition: ret is the hights block number that we have seen so far
 */
long eth_blockNumber()
{
    unsigned long ret;
    std::string blk = c.eth_blockNumber();
    std::stringstream ss;
    ss << std::hex << blk;
    ss >> ret;
    return ret;
}
/*Creates a Request block, from inputer data this is used to parse the data from the block
*/ 
Request::Request(uint8_t *input) {
    this->id = u64_from_b(input + 0x20 - sizeof(this->id));
    this->type = u64_from_b(input + 0x40 - sizeof(this->type));

    memcpy(this->requester, input + 0x40 + 16, 20);

    this->fee = u32_from_b(input + 0x80 - sizeof(this->fee));
    memcpy(this->callback, input + 0x80 + 16, 20);

    memcpy(this->param_hash, input + 0xa0, 32);

    //timestamp
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
/*Destructor */
Request::~Request() {
    if (this->data != NULL) {
        free(this->data);
    }
}
