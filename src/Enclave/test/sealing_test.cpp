//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include <sgx_tseal.h>
#include "../Enclave_t.h"
#include "commons.h"
#include "debug.h"
#include "mbedtls/ecdsa.h"

extern "C" int seal_data_test();
int seal_data_test()
{
    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);
    mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_SECP256K1);
    mbedtls_mpi_read_string(&ctx.d, 16, "cd244b3015703ddf545595da06ada5516628c5feadbf49dc66049c4b370cc5d8");
    mbedtls_ecp_mul( &ctx.grp, &ctx.Q, &ctx.d, &ctx.grp.G, NULL, NULL );

    uint8_t x[1024*10];
    uint32_t len = sgx_calc_sealed_data_size(0, sizeof(x));
    sgx_sealed_data_t *p = (sgx_sealed_data_t *)malloc(len);
    if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -10;

    if (sgx_seal_data(0, 0, sizeof(x), x, len, p) != SGX_SUCCESS) return -1;
    uint32_t MACtext_length = sgx_get_add_mac_txt_len(p);
    uint32_t decrypted_text_length = sgx_get_encrypt_txt_len(p);
    uint8_t y[decrypted_text_length];
    if (sgx_unseal_data(p, 0, &MACtext_length, y, &decrypted_text_length) != SGX_SUCCESS) return -1;
    if (MACtext_length != 0) return -1;
    if (decrypted_text_length != sizeof(x)) return -1;
    if (memcmp(x, y, sizeof(x))) {
        hexdump("x:", x, sizeof(x));
        hexdump("y:", y, sizeof(x));
        return -5;
    }
    free(p);
    return 0;
}
