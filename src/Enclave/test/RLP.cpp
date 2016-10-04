#include "Transaction.h"

extern "C" int RLP_self_test();
int RLP_self_test()
{
    TX tx(TX::MessageCall);
    return 0;
}
