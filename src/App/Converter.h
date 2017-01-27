#ifndef TC_LINUX_CONVERTER_H
#define TC_LINUX_CONVERTER_H

#include <string>
#include <vector>

void hexToBuffer(const std::__cxx11::string &str, unsigned char *buffer, size_t bufSize);
void hexToBuffer(const std::__cxx11::string &hex, std::vector<uint8_t> &buffer);
std::__cxx11::string bufferToHex(const unsigned char *buffer, size_t bufSize, bool prefix = false);
#endif //TC_LINUX_CONVERTER_H
