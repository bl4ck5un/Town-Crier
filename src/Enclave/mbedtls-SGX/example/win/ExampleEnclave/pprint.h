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

#include "stddef.h"

#ifndef DEBUG_H_
#define DEBUG_H_

#define SAMPLE_STEAM_REQUEST \
"0x" \
"0000000000000000000000000000000000000000000000000000000000000001" \
"0000000000000000000000000000000000000000000000000000000000000002" \
"000000000000000000000000c948866170205d12afb7c782c9f65551369bdb16" \
"0000000000000000000000000000000000000000000000000009c51c4521e000" \
"000000000000000000000000c948866170205d12afb7c782c9f65551369bdb16" \
"5f4997b9fe6f4a360a7f498270dc11f8c008e275915a36c9614c304f2686259f" \
"00000000000000000000000000000000000000000000000000000000000000e0" \
"0000000000000000000000000000000000000000000000000000000000000006" \
"6465616462656566000000000000000000000000000000000000000000000000" \
"6465616462656566000000000000000000000000000000000000000000000000" \
"706c616365686f6c646572000000000000000000000000000000000000000000" \
"000000000000000000000000000000000000000000000000000000000000003c" \
"0000000000000000000000000000000000000000000000000000000000000001" \
"506f7274616c0000000000000000000000000000000000000000000000000000"

#if defined(__cplusplus)
extern "C" {
#endif

void dump_buf( const char *title, unsigned char *buf, size_t len );
void hexdump(const char* title, void const * data, unsigned int len);
void string_dump(const char* title, void const* data, size_t len);


#if defined(__cplusplus)
}
#endif



#endif