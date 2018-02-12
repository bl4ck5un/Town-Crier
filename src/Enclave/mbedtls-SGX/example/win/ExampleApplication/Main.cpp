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
// the TowCrier source code. No other rights to use TownCrier and its
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

#include <iostream>
#include <stdio.h>
#include "ExampleEnclave_u.h"

#include "Utils.h"

#include "sgx_urts.h"

using namespace std;

/* Global EID shared by multiple threads */
sgx_enclave_id_t eid = 0;


int server()
{
    int ret;
    sgx_accept(eid, &ret);
    return ret;
}

int client()
{
    int ret = 0;
    sgx_connect(eid, &ret);
    return ret;
}

int main()
{
    int ret;

#if defined(_MSC_VER)
    if (query_sgx_status() < 0) {
        cout << "sgx is not support" << endl;
        ret = -1; 
        goto exit;
    }
#endif

    ret = initialize_enclave(&eid);

/* 
    // a simpler way to initialize an enclave
    int updated = 0;
    sgx_launch_token_t token = {0};
    sgx_status_t st;

    st = sgx_create_enclavea(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
    if (st != SGX_SUCCESS)
    {
        print_error_message(st);
        printf("failed to create enclave. Returned %#x", st);
        goto exit;
    }
*/
    if (ret != 0)
    {
        goto exit;
    }
    printf("Enclave %llu created\n", eid);
    client();

exit:
    printf("%%Info: all enclave closed successfully.\n");
    printf("%%Enter a character before exit ...\n");
    system("pause");
    return 0;
}