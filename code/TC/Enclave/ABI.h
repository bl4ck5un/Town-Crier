#pragma once
#include "Commons.h"
#include <cstddef>
#include <vector>

using namespace std;

#pragma warning (push)
#pragma warning (disable: 4512)

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
//    size_t head_len() override {return this->encode_len();}
//    size_t tail_len() override {return 0;}
    int encode(bytes& out) override;
    int encode_len() override {return 32;}
    bool dynamic() override {return false;}
    ~ABI_UInt64() {};
};

class ABI_UInt8: public ABI_UInt64
{
public:
    ABI_UInt8(uint8_t data): ABI_UInt64(data) {};
    int encode(bytes& out) override;
    ~ABI_UInt8() {};
};

class ABI_UInt32: public ABI_UInt64
{
public:
    ABI_UInt32(uint32_t data): ABI_UInt64(data) {};
    int encode(bytes& out) override;
    ~ABI_UInt32() {};
};

class ABI_Address: public ABI_serializable
{
protected:
    bytes20* _data;
public:
    ABI_Address(bytes20* data): _data(data) {};
    int encode(bytes& out) override;
    int encode_len() override {return 32;}
    bool dynamic() override {return false;}
    ~ABI_Address() {};
};


class ABI_Bytes32: public ABI_serializable
{
protected:
    bytes32* _data;
public:
//    size_t head_len() override {return this->encode_len();}
//    size_t tail_len() override {return 0;}
    ABI_Bytes32(bytes32* data): _data(data) {};
    int encode(bytes& out) override;
    int encode_len() override {return 32;}
    bool dynamic() override {return false; }
    ~ABI_Bytes32() {};
};

class ABI_Bytes: public ABI_serializable
{
protected:
    bytes& _data;
public:
    ABI_Bytes(bytes& data) : _data(data){};
//    size_t head_len() override {return 32;}
//    size_t tail_len() override {return this->encode_len();}
    int encode(bytes& out) override;
    int encode_len() override {return 32 + ROUND_TO_32(this->_data.size());}
    bool dynamic() override {return true;}
    ~ABI_Bytes() {};
};

class ABI_T_Array: public ABI_serializable
{
protected:
    vector<ABI_serializable*>& items;
public:
    ABI_T_Array(vector<ABI_serializable*>& items): items(items)
    {
        for (size_t i = 0; i < items.size(); i++)
        {
            if (items[i]->dynamic())
                throw std::invalid_argument("item is dynamic");
        }
    }
    int encode(bytes& out) override;
    int encode_len() override;
    bool dynamic() override {return true;}
    ~ABI_T_Array() {};
};

class ABI_Generic_Array: public ABI_serializable
{
protected:
    vector<ABI_serializable*>& items;
public:
//    size_t head_len() override {return 32;}
//    size_t tail_len() override;
    ABI_Generic_Array(vector<ABI_serializable*>& items) : items(items) {};
    int encode(bytes& out) override;
    int encode_len() override;
    bool dynamic() override {return true;}
    ~ABI_Generic_Array() {};
};

#pragma warning(pop)

//request(uint8 type, address cb, bytes4 cb_fid, bytes32[] req)
class ABI_Reader_adhoc
{
protected:
    string type;
    string callback;
    string callbackFID;
    string requestData;
public:
    ABI_Reader_adhoc(string abi)
    {
        type = abi.substr(0, 32);
        callback = abi.substr(32, 64);
        callbackFID = abi.substr(64, 96);
        requestData = abi.substr(96, abi.length());
    }
};

int get_demo_ABI();

int ABI_self_test();