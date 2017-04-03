//
// Created by fanz on 10/14/16.
//

#ifndef TOWN_CRIER_ENCODING_H
#define TOWN_CRIER_ENCODING_H

#include <vector>
#include "Log.h"

class bytes : public std::vector<uint8_t> {
protected:
	void rlp(bytes& out, unsigned len);
public:
    bytes(){}
    bytes(int len):std::vector<uint8_t>(len, static_cast<uint8_t>(0)){}
    bytes(std::vector<uint8_t> data):std::vector<uint8_t>(data){}
    virtual void from_hex(const char *src);
	virtual void to_rlp(bytes &out);

    void appendInt(uint64_t in, uint8_t byteLen);
};

class bytes32 : public bytes{
	unsigned _size;
public:
	bytes32():bytes(32){_size = 0;}
	unsigned size() const {return _size;}
	void set_size(int size) {_size = size;}
	void from_hex(const char *src);
	void to_rlp(bytes &);
};

uint8_t get_n_th_byte (uint64_t in, int n);
int append_as_uint256(bytes &out, uint64_t in, int len);
uint8_t bytesRequired(int _i);

#endif //TOWN_CRIER_ENCODING_H
