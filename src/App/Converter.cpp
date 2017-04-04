#include <boost/algorithm/hex.hpp>

#include "Converter.h"

using namespace std;

void hexToBuffer(const string &str, unsigned char *buffer, size_t bufSize) {
  if (buffer == nullptr)
    throw invalid_argument("buffer is null");
  if (str.size() == 0)
    return;

  auto offset = (str.compare(0, 2, "0x") == 0) ? 2 : 0;
  if ((str.size() - offset) / 2 > bufSize) {
    throw invalid_argument("buffer is too small");
  }
  boost::algorithm::unhex(str.begin() + offset, str.end(), buffer);
}

void hexToBuffer(const string &hex, vector<uint8_t> &buffer) {
  if (hex.size() == 0) {
    buffer.clear();
    return;
  }

  auto offset = (hex.compare(0, 2, "0x") == 0) ? 2 : 0;
  boost::algorithm::unhex(hex.begin() + offset, hex.end(), back_inserter(buffer));
}

string bufferToHex(const unsigned char *buffer, size_t bufSize, bool prefix) {
  string hex;
  if (prefix) {
    hex += "0x";
  }
  boost::algorithm::hex(buffer, buffer + bufSize, back_inserter(hex));
  return hex;
}

string bufferToHex(const vector<unsigned char>& buffer, bool prefix) {
  string hex;
  if (prefix) {hex += "0x";}
  boost::algorithm::hex(buffer.begin(), buffer.end(), back_inserter(hex));
  return hex;
}
