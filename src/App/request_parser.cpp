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

#include "App/request_parser.h"

#include <stdint.h>

#include <iostream>
#include <sstream>

#include "App/converter.h"
#include "App/utils.h"
#include "Common/Constants.h"
#include "App/logging.h"
#include "App/debug.h"

using tc::RequestParser;
using std::stringstream;
using std::invalid_argument;
using std::out_of_range;
using std::strtol;
using std::stoi;

namespace tc {
namespace requestParser {
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("request_parser.cpp"));
}
}

using tc::requestParser::logger;

inline static unsigned int __hextoi(const string &str) {
  return static_cast<unsigned int>(stoi(str, nullptr, 16));
}

inline static uint64_t __hextol(const string &str) {
  return stoul(str, nullptr, 16);
}

void RequestParser::valueOf(const std::string &input, const std::string &hash) {
  this->tx_hash = hash;

  LL_LOG("request parser get is %s", input.c_str());
  if (input.size() < RequestParser::REQUEST_MIN_LEN) {
    throw RequestParserException("input string is too short");
  }

  size_t offset = 0;
  try {
    // 0x00 - 0x20 bytes : id
    offset = (input.compare(0, 2, "0x") == 0) ? 2 : 0;
    this->id = __hextol(input.substr(offset, ENTRY_LEN));
    offset += ENTRY_LEN;

    // 0x20 - 0x40 bytes : requestType
    this->type = __hextol(input.substr(offset, ENTRY_LEN));
    offset += ENTRY_LEN;

    // 0x40 - 0x60 bytes : requester
    offset += ADDRESS_LEADING_ZERO;  // skipping leading zeroes
    hexToBuffer(input.substr(offset, ADDRESS_LEN), this->requester,
                sizeof this->requester);
    offset += ADDRESS_LEN;

    // 0x60 - 0x80       : fee
    this->fee = __hextol(input.substr(offset, ENTRY_LEN));
    offset += ENTRY_LEN;

    // 0x80 - 0xa0       : cb
    offset += ADDRESS_LEADING_ZERO;  // skipping leading zeroes
    hexToBuffer(input.substr(offset, ADDRESS_LEN), this->callback,
                sizeof this->callback);
    offset += ADDRESS_LEN;

    // 0xa0 - 0xc0       : hash
    hexToBuffer(input.substr(offset, ENTRY_LEN), this->param_hash,
                sizeof this->param_hash);
    offset += ENTRY_LEN;

    // 0xc0 - 0xe0       : timestamp
    this->timestamp = __hextol(input.substr(offset, ENTRY_LEN));
    offset += ENTRY_LEN;

    // 0xe0 - 0x100      : offset of requestData
    offset += ENTRY_LEN;  // skipping offset

    // 0x100 - 0x120     : reqLen (in bytes32)
    this->data_len = __hextoi(input.substr(offset, ENTRY_LEN)) * 32;
    offset += ENTRY_LEN;
  }
  catch (const std::out_of_range &e) {
    LL_CRITICAL("bad request: %s", e.what());
    throw RequestParserException(e.what());
  }
  catch (const std::exception &e) {
    LL_CRITICAL("bad request: %s", e.what());
    throw RequestParserException(e.what());
  }
  catch (...) {
    LL_CRITICAL("bad request");
    throw RequestParserException("unknown exception thrown");
  }

  // 0x120 - ...       : reqData
  if (this->data_len > TC_REQUEST_PAYLOAD_LIMIT) {
    throw invalid_argument("request data is too large");
  }

  hexToBuffer(input.substr(offset), &this->data);
}

RequestParser::RequestParser(const string &input, const string &hash)
    : raw_request(input) {
  this->valueOf(input, hash);
}

RequestParser::~RequestParser() {}

const string &RequestParser::getRawRequest() const { return raw_request; }

const string RequestParser::toString() const {
  stringstream ss;
  ss << "request id=" << this->id << " type=" << this->type << " with date "
     << this->data_len << "B";
  return ss.str();
}
uint64_t RequestParser::getId() const { return id; }
uint64_t RequestParser::getType() const { return type; }
const uint8_t *RequestParser::getRequester() const { return requester; }
uint64_t RequestParser::getFee() const { return fee; }
const uint8_t *RequestParser::getCallback() const { return callback; }
const uint8_t *RequestParser::getParamHash() const { return param_hash; }
uint64_t RequestParser::getTimestamp() const { return timestamp; }
size_t RequestParser::getDataLen() const { return data_len; }
const uint8_t * RequestParser::getData() const { return data.data(); }
size_t RequestParser::getRequesterLen() { return sizeof requester; }
size_t RequestParser::getCallbackLen() { return sizeof callback; }
size_t RequestParser::getParamHashLen() { return sizeof param_hash; }
const string &RequestParser::getTransactionHash() const { return tx_hash; }

void RequestParser::dumpData() const {
  hexdump("data", getData(), getDataLen());
}
