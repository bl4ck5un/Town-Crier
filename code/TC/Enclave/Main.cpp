#include "ABI.h"
#include "Main.h"

int Test_main()
{
    int ret;
    ret = ABI_self_test();
    if (ret)
    {
        LL_CRITICAL("%s failed. Returned %d.", "ABI_self_test", ret);
    }
    else
    {
        LL_CRITICAL("%s passed. Returned %d.", "ABI_self_test", ret);
    }
    return 0;
}