/* * Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
 *
 * Permission to use the "TownCrier" software ("TownCrier"), officially
 * docketed at the Center for Technology Licensing at Cornell University
 * as D-7364, developed through research conducted at Cornell University,
 * and its associated copyrights solely for educational, research and
 * non-profit purposes without fee is hereby granted, provided that the
 * user agrees as follows:
 *
 * The permission granted herein is solely for the purpose of compiling
 * the TownCrier source code. No other rights to use TownCrier and its
 * associated copyrights for any other purpose are granted herein,
 * whether commercial or non-commercial.
 *
 * Those desiring to incorporate TownCrier software into commercial
 * products or use TownCrier and its associated copyrights for commercial
 * purposes must contact the Center for Technology Licensing at Cornell
 * University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
 * ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
 * commercial license.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
 * ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
 * UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
 * REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
 * OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
 * OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
 * PATENT, TRADEMARK OR OTHER RIGHTS.
 *
 * TownCrier was developed with funding in part by the National Science
 * Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
 * CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
 * Google Faculty Research Awards, and a VMWare Research Award.
 */

/*
 * Implementation of Ethereum ABI
 * https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
 */

#ifndef TC_ENCLAVE_ETH_ABI_H
#define TC_ENCLAVE_ETH_ABI_H

#include "commons.h"
#include "encoding.h"
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
    explicit ABI_Bytes32(bytes32* data): _data(data) {};
    int encode(bytes& out) override;
    int encode_len() override {return 32;}
    bool dynamic() override {return false; }
    ~ABI_Bytes32() override = default;
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

#endif
