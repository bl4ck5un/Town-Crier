//
// Created by fanz on 10/14/16.
//

#include "encoding.h"
#include "commons.h"

uint8_t get_n_th_byte (uint64_t in, int n)
{
    if (n > 8) {printf_sgx("n is too big\n"); return 0xFF;}
    return (in >> (8*n)) & 0xff;
}

/*!
 * encode in in big-endian order. Prepend zeros to make the result of 32-byte aligned.
 * @param out
 * @param in input int
 * @param len length of int in byte
 * @return
 */
int enc_int(bytes& out, uint64_t in, int len)
{
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

uint8_t bytesRequired(int _i)
{
    uint8_t i = 0;
    for (; _i != 0; ++i, _i >>= 8) {}
    return i;
}

void bytes::fromHex(const char* src)
{
    if (strlen(src) % 2 != 0)
    { LL_CRITICAL("Error: input is not of even len\n");}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        std::vector<uint8_t>::push_back(hex2int(*src)*16 + hex2int(src[1]));
        src += 2;
    }
}

void bytes::rlp(bytes& out, unsigned len)
{
    int i;
    size_t len_len;
    if (len == 1 && (std::vector<uint8_t>::operator[](0)) < 0x80 ) {
        out.push_back(std::vector<uint8_t>::operator[](0));
        return;
    }
    // longer than 1
    if (len < 56) {
        out.push_back(0x80 + static_cast<uint8_t>(len));
        for (i = 0; i < len; i++) out.push_back(std::vector<uint8_t>::operator[](i));
    }
    else {
        len_len = bytesRequired(len);
        if (len_len > 8) {throw std::invalid_argument("Error: len_len > 8");}
        out.push_back(0xb7 + static_cast<uint8_t>(len_len));

        for (i=len_len-1; i >=0; i--) out.push_back( static_cast<uint8_t>((len >> (8*i)) & 0xFF));
        for (i = 0; i < len; i++) out.push_back(std::vector<uint8_t>::operator[](i));
    }
}

void bytes::rlp(bytes& out)
{
	return rlp(out, std::vector<uint8_t>::size());
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
/*
    convert a hex string to a byte array
    [in]  src 
    [out] target 
    [out] len: how many bytes get written to the target
*/
void bytes32::fromHex(const char* src)
{
    _size = 0;
    if (strlen(src) % 2 != 0)
    { LL_CRITICAL("Error: input is not of even len\n");}
    if (strncmp(src, "0x", 2) == 0) src += 2;
    while(*src && src[1])
    {
        try { bytes::operator[](_size) = hex2int(*src)*16 + hex2int(src[1]); }
        catch (std::invalid_argument e)
		{ printf_sgx("Error: can't convert %s to bytes\n", src); }
        src += 2; 
        _size++;
    }
    if (_size == 1 && bytes::operator[](0) == 0) _size = 0;
}

void bytes32::rlp(bytes& out)
{
	bytes::rlp(out, _size);
}
