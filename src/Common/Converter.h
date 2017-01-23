//
// Created by fanz on 6/11/16.
//

#include <stdint.h>
#include <vector>
#include <string>

#ifndef TC_LINUX_CONVERTER_H
#define TC_LINUX_CONVERTER_H

void hexToBuffer(const ::std::string &str, unsigned char *buffer, size_t bufSize);
std::string bufferToHex(const unsigned char *buffer, size_t bufSize, bool prefix = false);

#endif //TC_LINUX_CONVERTER_H
