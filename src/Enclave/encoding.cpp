//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TowCrier source code. No other rights to use TownCrier and its
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

#include "encoding.h"
#include "commons.h"

uint8_t get_n_th_byte(uint64_t in, int n) {
  if (n > 8) {
    printf_sgx("n is too big\n");
    return 0xFF;
  }
  return (in >> (8 * n)) & 0xff;
}

/*!
 * encode in in big-endian order. Prepend zeros to make the result of 32-byte aligned.
 * @param out
 * @param in input int
 * @param len length of int in byte
 * @return
 */
int append_as_uint256(bytes &out, uint64_t in, int len) {
  if (len > 32) {
    printf_sgx("Error: too big\n");
    return -1;
  }
  // padding with 0
  for (int i = 0; i < 32 - len; i++) { out.push_back(0); }
  // push big-endian int
  for (int i = len - 1; i >= 0; i--) { out.push_back(get_n_th_byte(in, i)); }
  return 0;
}

uint8_t bytesRequired(int _i) {
  uint8_t i = 0;
  for (; _i != 0; ++i, _i >>= 8) {}
  return i;
}

bytes::bytes(bytes a, bytes b) {
  std::vector<uint8_t >::insert(std::vector<uint8_t>::end(), a.begin(), a.end());
  std::vector<uint8_t >::insert(std::vector<uint8_t>::end(), b.begin(), b.end());
}

void bytes::from_hex(const char *src) {
  if (strlen(src) % 2 != 0) { LL_CRITICAL("Error: input is not of even len\n"); }
  if (strncmp(src, "0x", 2) == 0) src += 2;
  while (*src && src[1]) {
    std::vector<uint8_t>::push_back(hex2int(*src) * 16 + hex2int(src[1]));
    src += 2;
  }
}

void bytes::rlp(bytes &out, unsigned len) {
  int i;
  size_t len_len;
  if (len == 1 && (std::vector<uint8_t>::operator[](0)) < 0x80) {
    out.push_back(std::vector<uint8_t>::operator[](0));
    return;
  }
  // longer than 1
  if (len < 56) {
    out.push_back(0x80 + static_cast<uint8_t>(len));
    for (i = 0; i < len; i++) out.push_back(std::vector<uint8_t>::operator[](i));
  } else {
    len_len = bytesRequired(len);
    if (len_len > 8) { throw std::invalid_argument("Error: len_len > 8"); }
    out.push_back(0xb7 + static_cast<uint8_t>(len_len));

    for (i = len_len - 1; i >= 0; i--) out.push_back(static_cast<uint8_t>((len >> (8 * i)) & 0xFF));
    for (i = 0; i < len; i++) out.push_back(std::vector<uint8_t>::operator[](i));
  }
}

void bytes::to_rlp(bytes &out) {
  return rlp(out, std::vector<uint8_t>::size());
}

bytes32::bytes32(uint64_t in) {
  // padding with 0
  size_t byteLen = sizeof in;
  std::vector<uint8_t >::insert(std::vector<uint8_t>::end(), 32 - byteLen, 0);
  // push big-endian int
  for (int i = byteLen - 1; i >= 0; i--) { std::vector<uint8_t>::push_back(get_n_th_byte(in, i)); }
}


bytes32::bytes32(std::string in) {
  if (in.length() > 32) {
    throw std::invalid_argument("too big");
  }
  // push string
  std::vector<uint8_t >::insert(std::vector<uint8_t>::end(), in.begin(), in.end());
  // padding with 0
  std::vector<uint8_t >::insert(std::vector<uint8_t>::end(), 32 - in.length(), 0x0);
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
/*
    convert a hex string to a byte array
    [in]  src 
    [out] target 
    [out] len: how many bytes get written to the target
*/
void bytes32::from_hex(const char *src) {
  _size = 0;
  if (strlen(src) % 2 != 0) { LL_CRITICAL("Error: input is not of even len\n"); }
  if (strncmp(src, "0x", 2) == 0) src += 2;
  while (*src && src[1]) {
    try { bytes::operator[](_size) = hex2int(*src) * 16 + hex2int(src[1]); }
    catch (std::invalid_argument e) { printf_sgx("Error: can't convert %s to bytes\n", src); }
    src += 2;
    _size++;
  }
  if (_size == 1 && bytes::operator[](0) == 0) _size = 0;
}

void bytes32::to_rlp(bytes &out) {
  bytes::rlp(out, _size);
}