#include "Transaction.h"
#include <vector>
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
    
    return 0;
}
