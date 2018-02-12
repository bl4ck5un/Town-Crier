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
 * the TownCrier source code. No other rights to use TownCrier and its
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

#include <sgx_tseal.h>
#include "stdint.h"

#include "commons.h"
#include "encoding.h"

#ifndef TX_H
#define TX_H

class Transaction {
 public:
  enum Type {
    NullTransaction,
    ContractCreation,
    MessageCall
  };

  Type m_type;
  bytes32 m_nonce;
  bytes32 m_gasPrice;
  bytes32 m_gas;
  bytes20 m_to;
  bytes32 m_value;
  bytes m_data;
  bytes32 r;
  bytes32 s;
  uint8_t v;
  Transaction(Type p,
              uint64_t nonce,
              uint64_t gasprice,
              uint64_t gaslimit,
              string to,
              uint64_t value,
              const bytes& data)
      : m_type(p), m_to(to.c_str()) {
    m_nonce.parseUInt64(nonce);
    m_gasPrice.parseUInt64(gasprice);
    m_gas.parseUInt64(gaslimit);
    m_value.parseUInt64(value);

    m_data = data;
  }
  void rlpEncode(bytes &out, bool withSig = true);
};

#if defined(__cplusplus)
extern "C" {
#endif

void rlp_item(const uint8_t *, int, bytes &) __attribute__ ((deprecated));
int form_transaction(int nonce,
                     uint64_t request_id,
                     uint8_t request_type,
                     const uint8_t *request_data,
                     size_t request_data_len,
                     uint64_t resp_error,
                     bytes resp_data,
                     uint8_t *tx_output_bf,
                     size_t *o_len,
                     bool sign = true);

#if defined(__cplusplus)
}
#endif
#endif
