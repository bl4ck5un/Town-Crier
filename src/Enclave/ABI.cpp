#include <stdint.h>
#include "ABI.h"
#include "Commons.h"
#include "keccak.h"
#include "Debug.h"

static uint8_t get_n_th_byte (uint64_t in, int n)
{
    if (n > 8) {printf_sgx("n is too big\n"); return 0xFF;}
    return (in >> (8*n)) & 0xff;
}

int enc_int(bytes& out, uint64_t in, int len)
{
    if (len > 32) {printf_sgx("Error: too big\n"); return -1; }
    // padding with 0
    for (int i = 0; i < 32 - len; i++)  {out.push_back(0); }
    // push big-endian int
    for (int i = len - 1; i >= 0; i--) {out.push_back(get_n_th_byte(in, i));}
    return 0;
}

int ABI_UInt64::encode(bytes& out)
{
    return enc_int(out, this->_data, 8);
}

int ABI_UInt8::encode(bytes& out)
{
    return enc_int(out, this->_data, 1);
}

int ABI_UInt32::encode(bytes& out)
{
    return enc_int(out, this->_data, 4);
}

int ABI_Address::encode(bytes& out)
{
    out.insert(out.end(), 12, 0);
    for (int i = 0; i < 20; i++)
    {
        out.push_back(this->_data->b[i]);
    }
    return 0;
}


int ABI_Bytes32::encode(bytes& out)
{
    for (int i = 0; i < 32; i++)
    {
        out.push_back(this->_data->b[i]);
    }
    return 0;
}

int ABI_Bytes::encode(bytes& out)
{
    if (enc_int(out, this->_data.size(), 4) != 0) {printf_sgx("Error! enc(out, int) return non-zero!\n"); return -1;}
    // padding left
    for (size_t i = 0; i < ROUND_TO_32(this->_data.size()) - this->_data.size(); i++) 
        {out.push_back(0);}
    out.insert(out.end(), this->_data.begin(), this->_data.end());
    return 0;  
}

int ABI_T_Array::encode(bytes& out)
{
    if (enc_int(out, this->items.size(), 4) != 0) {printf_sgx("Error! enc(out, int) return non-zero!\n"); return -1;}
    for (size_t i = 0; i < this->items.size(); i++)
    {
        if (items[i]->encode(out)) return -1;
    }
    return 0;
}

int ABI_T_Array::encode_len()
{
    int len = 0;
    for (size_t i = 0; i < items.size(); i++)
    {
        len += this->items[i]->encode_len();
    }

    return len;
}

int ABI_Generic_Array::encode(bytes& out)
{
    size_t i, j;
    size_t head_len_sum = 0;
    size_t tail_len_sum = 0;
    for (i = 0; i < this->items.size(); i++) { head_len_sum += this->items[i]->head_len();}

    // head encoding
    for (i = 0; i < this->items.size(); i++)
    {
        if (this->items[i]->dynamic())
        {
            for (j = 0; j < i; j++)
            {
                tail_len_sum += this->items[j]->tail_len();
            }
            if (enc_int(out, head_len_sum + tail_len_sum, sizeof (size_t)) != 0) {return -1;}
        }
        else
        {
            if (this->items[i]->encode(out)) {
                printf_sgx("Error\n");
                return -1;
            }
        }
    }

    // tail encoding
    for (i = 0; i < this->items.size(); i++)
    {
        if (this->items[i]->dynamic())
        {
            if (this->items[i]->encode(out)) {return -1;};
        }
    }

    return 0;    
}

int ABI_Generic_Array::encode_len()
{
    int len = 0;
    for (size_t i = 0; i < items.size(); i++)
    {
        len += this->items[i]->encode_len();
    }

    return len;
}
