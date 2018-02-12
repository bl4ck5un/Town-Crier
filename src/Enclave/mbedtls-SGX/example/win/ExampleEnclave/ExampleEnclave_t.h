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
 * the TowCrier source code. No other rights to use TownCrier and its
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

#ifndef EXAMPLEENCLAVE_T_H__
#define EXAMPLEENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "mbedtls/net_v.h"
#include "mbedtls/timing_v.h"

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


int sgx_connect();
int sgx_accept();

sgx_status_t SGX_CDECL ocall_mbedtls_net_connect(int* retval, mbedtls_net_context* ctx, const char* host, const char* port, int proto);
sgx_status_t SGX_CDECL ocall_mbedtls_net_bind(int* retval, mbedtls_net_context* ctx, const char* bind_ip, const char* port, int proto);
sgx_status_t SGX_CDECL ocall_mbedtls_net_accept(int* retval, mbedtls_net_context* bind_ctx, mbedtls_net_context* client_ctx, void* client_ip, size_t buf_size, size_t* ip_len);
sgx_status_t SGX_CDECL ocall_mbedtls_net_set_block(int* retval, mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL ocall_mbedtls_net_set_nonblock(int* retval, mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL ocall_mbedtls_net_usleep(unsigned long int usec);
sgx_status_t SGX_CDECL ocall_mbedtls_net_recv(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len);
sgx_status_t SGX_CDECL ocall_mbedtls_net_send(int* retval, mbedtls_net_context* ctx, const unsigned char* buf, size_t len);
sgx_status_t SGX_CDECL ocall_mbedtls_net_recv_timeout(int* retval, mbedtls_net_context* ctx, unsigned char* buf, size_t len, uint32_t timeout);
sgx_status_t SGX_CDECL ocall_mbedtls_net_free(mbedtls_net_context* ctx);
sgx_status_t SGX_CDECL ocall_print_string(int* retval, const char* str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
