//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

#include <iostream>
#include <sstream>

#include "request-parser.hxx"
#include "Log.h"
#include "Converter.h"
#include "utils.h"

using namespace tc;

inline static unsigned int __hextoi(const string &str) {
  try {
    return static_cast<unsigned int> (std::stoi(str, nullptr, 16));
  }
  catch (std::out_of_range &ex) {
    throw invalid_argument("Exception happen when calling stoi(" + str + ")");
  }
}

inline static unsigned long __hextol(const string &str) {
  return std::stoul(str, nullptr, 16);
}

RequestParser::RequestParser(const std::string &input) : raw_request(input) {
  LL_LOG("request parser get is %s", input.c_str());
  if (input.size() < RequestParser::REQUEST_MIN_LEN) {
    throw std::invalid_argument("input string is too short");
  }
  // 0x00 - 0x20 bytes : id
  size_t offset = (input.compare(0, 2, "0x") == 0) ? 2 : 0;
  this->id = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x20 - 0x40 bytes : requestType
  this->type = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x40 - 0x60 bytes : requester
  offset += ADDRESS_LEADING_ZERO; //skipping leading zeroes
  hexToBuffer(input.substr(offset, ADDRESS_LEN), this->requester, sizeof this->requester);
  offset += ADDRESS_LEN;

  // 0x60 - 0x80       : fee
  this->fee = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0x80 - 0xa0       : cb
  offset += ADDRESS_LEADING_ZERO; //skipping leading zeroes
  hexToBuffer(input.substr(offset, ADDRESS_LEN), this->callback, sizeof this->callback);
  offset += ADDRESS_LEN;

  // 0xa0 - 0xc0       : hash
  hexToBuffer(input.substr(offset, ENTRY_LEN), this->param_hash, sizeof this->param_hash);
  offset += ENTRY_LEN;

  // 0xc0 - 0xe0       : timestamp
  this->timestamp = __hextol(input.substr(offset, ENTRY_LEN));
  offset += ENTRY_LEN;

  // 0xe0 - 0x100      : offset of requestData
  offset += ENTRY_LEN; // skipping offset

  // 0x100 - 0x120     : reqLen (in bytes32)
  this->data_len = __hextoi(input.substr(offset, ENTRY_LEN)) * 32;
  offset += ENTRY_LEN;

  // 0x120 - ...       : reqData
  if (this->data_len > 102400) {
    throw std::invalid_argument("request data is too large");
  }

  this->data = static_cast<uint8_t *>(malloc(this->data_len));
  hexToBuffer(input.substr(offset), this->data, this->data_len);
}

RequestParser::~RequestParser() {
  if (this->data != NULL) {
    free(this->data);
  }
}
const string &RequestParser::getRawRequest() const {
  return raw_request;
}

const string RequestParser::toString() const {
  stringstream ss;
  ss << "request id=" << this->id << " type=" << this->type << " with date " << this->data_len << "B";
  return ss.str();
}
unsigned long RequestParser::getId() const {
  return id;
}
unsigned long RequestParser::getType() const {
  return type;
}
const uint8_t *RequestParser::getRequester() const {
  return requester;
}
unsigned long RequestParser::getFee() const {
  return fee;
}
const uint8_t *RequestParser::getCallback() const {
  return callback;
}
const uint8_t *RequestParser::getParamHash() const {
  return param_hash;
}
unsigned long RequestParser::getTimestamp() const {
  return timestamp;
}
size_t RequestParser::getDataLen() const {
  return data_len;
}
uint8_t *RequestParser::getData() const {
  return data;
}
