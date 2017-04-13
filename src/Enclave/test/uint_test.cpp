#include "commons.h"
#include <inttypes.h>

int uint_utils_test() {
  bool results[3] = {false};

  uint64_t eight_bytes = 0x1122334455667788;
  uint64_t _swapped = swap_endian<uint64_t>(eight_bytes);
  results[0] = _swapped == 0x8877665544332211;

  uint32_t four_bytes = 0x11223344;
  uint32_t _32swapped = swap_endian<uint32_t>(four_bytes);
  results[1] = _32swapped == 0x44332211;

  unsigned char buf[32] = {0};
  for (int i = 0; i < sizeof buf; i++) {
    buf[i] = (unsigned char) i;
  }

  uint64_t _from_buf = uint_bytes<uint64_t, 32>(buf);
  uint64_t _expected_from_buf = 0x18191a1b1c1d1e1f;
  results[2] = _from_buf == _expected_from_buf;
  if (!results[2]) {
    LL_CRITICAL("expected: %" PRIx64 ", get: %" PRIx64, _expected_from_buf, _from_buf);
  }
  bool final = true;
  for (int i = 0; i < sizeof results; i++) {
    final &= results[i];
    if (!results[i]) {
      LL_CRITICAL("test %d failed", i);
    }
  }

  return final ? 0 : 1;
}