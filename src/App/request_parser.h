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

#ifndef SRC_APP_REQUEST_PARSER_H_
#define SRC_APP_REQUEST_PARSER_H_

#include <cstdint>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace tc {
class RequestParser {
  // RequestInfo(uint64 id, uint8 requestType, address requester, uint fee,
  // address callbackAddr, bytes32 paramsHash,
  // bytes32 timestamp, bytes32[] requestData);

  // Byte code of ABI encoding:
  // 0x00 - 0x20 bytes : id
  // 0x20 - 0x40 bytes : requestType
  // 0x40 - 0x60 bytes : requester
  // 0x60 - 0x80       : fee
  // 0x80 - 0xa0       : cb
  // 0xa0 - 0xc0       : hash
  // 0xc0 - 0xe0       : timestamp
  // 0xe0 - 0x100       : offset of requestData
  // 0x100 - 0x120      : reqLen (in bytes32)
  // 0x120 - ...       : reqData
 public:
  static const int REQUEST_MIN_LEN = 2 * 120;
  static const int ENTRY_LEN = 2 * 32;
  static const int ADDRESS_LEN = 2 * 20;
  static const int ADDRESS_LEADING_ZERO = 2 * 12;

 private:
  const string raw_request;
  uint64_t id;
  uint64_t type;
  uint8_t requester[20];
  uint64_t fee;
  uint8_t callback[20];
  uint8_t param_hash[32];
  uint64_t timestamp;
  size_t data_len;
  vector<uint8_t> data;

  string tx_hash;

 public:
  RequestParser() {}
  explicit RequestParser(const string &input, const string &hash);
  ~RequestParser();

  void valueOf(const string &input, const string &hash);

  uint64_t getId() const;
  uint64_t getType() const;
  const uint8_t *getRequester() const;
  uint64_t getFee() const;
  const uint8_t *getCallback() const;
  const uint8_t *getParamHash() const;
  uint64_t getTimestamp() const;
  size_t getDataLen() const;
  const uint8_t * getData() const;
  const string &getRawRequest() const;
  const string &getTransactionHash() const;
  const string toString() const;
  void dumpData() const;

  size_t getRequesterLen();
  size_t getCallbackLen();
  size_t getParamHashLen();
};
}  // namespace tc

class RequestParserException : public std::exception {
  string reason;
 public:
  explicit RequestParserException(const string &what) { reason = what; }
  char const *what() const noexcept override { return reason.c_str(); }
};

#endif  // SRC_APP_REQUEST_PARSER_H_
