#ifndef TC_LINUX_CONVERTER_H
#define TC_LINUX_CONVERTER_H

#include <string>
#include <vector>

using namespace std;

void hexToBuffer(const string &str, unsigned char *buffer, size_t bufSize);
void hexToBuffer(const string &hex, vector<uint8_t> &buffer);
string bufferToHex(const unsigned char *buffer, size_t bufSize, bool prefix = false);
string bufferToHex(vector<unsigned char> const &  buffer, bool prefix = false);
#endif //TC_LINUX_CONVERTER_H
