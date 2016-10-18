/*
 * Implementation of Ethereum ABI
 * https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
 *
 * This file is part of Town Crier
 */
#pragma once
#include "Commons.h"
#include "Encoding.h"
#include <cstddef>
#include <vector>

using namespace std;

class ABI_serializable
{
public:
    virtual size_t head_len() {return dynamic() ? 32 : this->encode_len();};
    virtual size_t tail_len() {return dynamic() ? this->encode_len() : 0; };
    virtual int encode(bytes& out) = 0;
    virtual int encode_len() = 0;
    virtual bool dynamic() = 0;
    virtual ~ABI_serializable() {};
};

class ABI_UInt64: public ABI_serializable
{
protected:
    uint64_t _data;
public:
    ABI_UInt64(uint64_t data): _data(data) {};
    int encode(bytes& out) ;
    int encode_len()  {return 32;}
    bool dynamic()  {return false;}
    ~ABI_UInt64() {};
};

class ABI_UInt8: public ABI_UInt64
{
public:
    ABI_UInt8(uint8_t data): ABI_UInt64(data) {};
    int encode(bytes& out) ;
    ~ABI_UInt8() {};
};

class ABI_UInt32: public ABI_UInt64
{
public:
    ABI_UInt32(uint32_t data): ABI_UInt64(data) {};
    int encode(bytes& out) ;
    ~ABI_UInt32() {};
};

class ABI_Address: public ABI_serializable
{
protected:
    bytes32* _data;
public:
    ABI_Address(bytes32* data): _data(data) {};
    int encode(bytes& out) ;
    int encode_len()  {return 32;}
    bool dynamic()  {return false;}
    ~ABI_Address() {};
};


class ABI_Bytes32: public ABI_serializable
{
protected:
    bytes32* _data;
public:
    ABI_Bytes32(bytes32* data): _data(data) {};
    int encode(bytes& out) ;
    int encode_len()  {return 32;}
    bool dynamic()  {return false; }
    ~ABI_Bytes32() {};
};

class ABI_Bytes: public ABI_serializable
{
protected:
    bytes& _data;
public:
    ABI_Bytes(bytes& data) : _data(data){};
    int encode(bytes& out) ;
    int encode_len()  {return 32 + ROUND_TO_32(this->_data.size());}
    bool dynamic()  {return true;}
    ~ABI_Bytes() {};
};

//template<class T>
//class ABI_T_Array: public ABI_serializable
//{
//protected:
//    vector<T*>& items;
//public:
//    ABI_T_Array(vector<T*>& items): items(items)
//    {
//        for (size_t i = 0; i < items.size(); i++)
//        {
//            if (items[i]->dynamic())
//                throw std::invalid_argument("item is dynamic");
//        }
//    }
//    int encode(bytes& out) ;
//    int encode_len() ;
//    bool dynamic()  {return true;}
//    ~ABI_T_Array() {};
//};

class ABI_Generic_Array: public ABI_serializable
{
protected:
    vector<ABI_serializable*>& items;
public:
    ABI_Generic_Array(vector<ABI_serializable*>& items) : items(items) {};
    int encode(bytes& out);
    int encode_len();
    bool dynamic() {return true;}
    ~ABI_Generic_Array() {};
};

