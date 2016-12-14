#include <gtest/gtest.h>

#include "../Init.h"
#include "../Enclave_u.h"

#include "../EthRPC.h"
#include "Converter.h"
#include <vector>

TEST (SSL, client) {

    sgx_enclave_id_t eid;
    int ret = initialize_enclave(ENCLAVE_FILENAME, &eid);
    ASSERT_EQ(SGX_SUCCESS, ret);

    ssl_self_test(eid, &ret);
    int ret = 0;
 	ret += UPS_scaper_test();
	ret += Bloomberg_scaper_test();
	ret += Google_scraper_test();
	ret += Yahoo_scraper_test();
    ret += test_get_page_on_sll();
	
    //Assert that all test cases pass
    ASSERT_EQ(0, ret);
}