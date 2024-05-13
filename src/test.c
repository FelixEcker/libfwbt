#include "fwbt.h"

#include <stdio.h>

#include "../test.fwbt.h"

int main(void) {
  fwbt_t fwbt;
  fwbt_error_t err = fwbt_parse_bytes(test_fwbt, test_fwbt_len, &fwbt);

  if (err != FWBT_OK) {
    goto exit;
  }

  fprintf(stderr, "Header Info:\n");
  fprintf(stderr, "\tSignature: %.*s ; Version: %u\n", 4, fwbt.header.signature,
          fwbt.header.version);
  fprintf(stderr, "\tKey Width: %u ; Value Width: %u ; Entry Count: %u\n",
          fwbt.header.key_width, fwbt.header.value_width,
          fwbt.header.entry_count);

  size_t outsize = 0;
  uint8_t *out;

  err = fwbt_serialize(fwbt, &out, &outsize);
  if (err != FWBT_OK) {
    goto exit;
  }

  fprintf(stderr, "outsize = %zu\n", outsize);
#ifdef WRITE_SERIALIZED
  fwrite(out, outsize, 1, stdout);
#endif

exit:
  return err;
}
