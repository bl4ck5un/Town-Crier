//
// Copyright (c) 2016-2017 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially docketed at
// the Center for Technology Licensing at Cornell University as D-7364, developed
// through research conducted at Cornell University, and its associated copyrights
// solely for educational, research and non-profit purposes without fee is hereby
// granted, provided that the user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling the
// TowCrier source code. No other rights to use TownCrier and its associated
// copyrights for any other purpose are granted herein, whether commercial or
// non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial products or use
// TownCrier and its associated copyrights for commercial purposes must contact the
// Center for Technology Licensing at Cornell University at 395 Pine Tree Road,
// Suite 310, Ithaca, NY 14850; email: ctl-connect@cornell.edu; Tel: 607-254-4698;
// FAX: 607-254-5454 for a commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
// ARISING OUT OF THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS, EVEN IF
// CORNELL UNIVERSITY MAY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO
// OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
// MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO REPRESENTATIONS AND EXTENDS NO
// WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE, OR THAT THE USE OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT
// INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science Foundation
// (NSF grants CNS-1314857, CNS-1330599, CNS-1453634, CNS-1518765, CNS-1514261), a
// Packard Fellowship, a Sloan Fellowship, Google Faculty Research Awards, and a
// VMWare Research Award.
//

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Debug.h"
#include "tls_client.h"
#include "../scrapers/steam2.h"
#include "Log.h"

int steam_self_test() {
  err_code err;
  int ret;
  SteamScraper testScraper;

  // api_key [string] (padded with \0)
  string api_key = "7978F8EDEF9695B57E72EC468E5781AD";
  api_key.append(64 - api_key.length(), 0x00);

  // buyer_id [string] (padded with \0)
  string buyer_id = "32884794";
  buyer_id.append(32 - buyer_id.length(), 0x00);

  // cutoff time [long] (big endian, pre-padded with 0)
  string cutoff_time = "1456380265";
  cutoff_time.append(32 - cutoff_time.length(), 0x00);

  // same as cutoff
  string item_len = "1";
  item_len.append(32 - item_len.length(), 0x00);

  string items = "Portal";
  items.append(32 - items.length(), 0x00);

  string request_data = api_key + buyer_id + cutoff_time + item_len + items;
  dump_buf("Request", (unsigned char*) request_data.c_str(), request_data.length());
  err = testScraper.handler((uint8_t *) request_data.c_str(), request_data.length(), &ret);
  return (err == NO_ERROR && ret == 1) ? 0 : 1;
}

