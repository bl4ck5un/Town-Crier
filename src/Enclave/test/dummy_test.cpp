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

#include <stdio.h>
#include "hybrid_cipher.h"
#include "env.h"

using namespace std;

int dummy_test() {
  string a = g_environment->get("a");
  LL_CRITICAL("a => %s", a.c_str());
  if (a != "env") {
    return -1;
  }

  HybridEncryption encrypt;
  try {
    ECPointBuffer server_pubkey;

    encrypt.queryPubkey(server_pubkey);
    hexdump("server public key", server_pubkey, sizeof(ECPointBuffer));

    string user_secret = "user_secret";

    hexdump("input", user_secret.data(), user_secret.size());

    string cipher_b64 = encrypt.hybridEncrypt(server_pubkey,
                                              reinterpret_cast<const uint8_t *>(user_secret.data()),
                                              user_secret.size());

    printf_sgx("ciphertext (base64): %s\n", cipher_b64.c_str());

    HybridCiphertext ciphertext = encrypt.decode(cipher_b64);

    vector<uint8_t> cleartext;
    encrypt.hybridDecrypt(ciphertext, cleartext);

    hexdump("decrypted", &cleartext[0], cleartext.size());
    return memcmp(&cleartext[0], &user_secret[0], cleartext.size());
  }
  catch (const exception &e) {
    LL_CRITICAL("%s", e.what());
    return -1;
  }
  catch (...) {
    LL_CRITICAL("Unknown exception");
    return -1;
  }

}
