#include <stdint.h>
#include <vector>

#include "Transaction.h"
#include "Debug.h"
#include "Encoding.h"
#include "Constants.h"

extern "C" int transaction_rlp_test();
int transaction_rlp_test()
{
    bytes v, t;
    t.push_back('d');
    t.push_back('o');
    t.push_back('g');
    rlp_item(&t[0], 3, v);
    if (v.size() != 4 || v[0] != 0x83) return -1;
    if (v[1] != 'd' || v[2] != 'o' || v[3] != 'g') return -1;
    t.clear();v.clear();
    rlp_item(&t[0], 0, v);
    if (v.size() != 1 || v[0] != 0x80) return v[0];
    
	int nonce = 0;
	int nonce_len = 32;
	uint64_t request_id = 1;
	uint8_t request_type = 2;
	int req_len = 192;
	uint8_t req_data[200] = {246, 141, 42, 50, 207, 23, 177, 49, 44, 109, 179, 242, 54, 163, 140, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 76, 159, 146, 246, 236, 30, 42, 32, 161, 65, 61, 10, 193, 184, 103, 163, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 92, 131, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 80, 111, 114, 116, 97, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int resp_len = 32;
	uint8_t resp_data[40] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int o_len;
	uint8_t serialized_tx[500];
	
	int ret = get_raw_signed_tx(nonce, nonce_len, 
							request_id, request_type,
							req_data, req_len,
							resp_data, resp_len,
							serialized_tx, &o_len);
    if (ret) return ret;
    if (o_len != 204) return 1;
	uint8_t ans[] = {248, 202, 128, 133, 11, 164, 59, 116, 0, 131, 1, 95, 144, 148, 136, 203, 90, 183, 19, 87, 217, 140, 123, 249, 202, 18, 3, 22, 197, 122, 67, 56, 15, 40, 128, 184, 100, 176, 112, 185, 186, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 18, 120, 47, 194, 110, 22, 175, 204, 97, 158, 123, 124, 230, 84, 174, 112, 89, 153, 10, 80, 130, 49, 109, 143, 251, 41, 135, 225, 230, 105, 64, 202, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28};

    bytes tcAddress;
    tcAddress.fromHex(TC_ADDRESS);

    for (int i = 14; i < 14 + 20; i++)
    {
        ans[i] = tcAddress.at(i-14);
    }


	if (memcmp(serialized_tx, ans, 136))
	{
        LL_CRITICAL("memcmp failed");
        hexdump("correct:", ans, 136);
        hexdump("Ours: ", serialized_tx, 136);
		return 1;
	}
	
    return 0;
}