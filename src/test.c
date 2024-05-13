#include "fwbt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../test.fwbt.h"

/* Writes the final state of the FWBT to stdout */
#define WRITE_SERIALIZED

/* macro for converting integer literal to string literal */
#define _STR(x) #x
#define STR(x) _STR(x)

/* Used to check for errors and log encountered ones. Exits with error code */
#define CHECK_ERR(x)                                                           \
  if (x != FWBT_OK) {                                                          \
    fprintf(stderr, STR(__LINE__) ": error %d\n", x);                          \
    exit(x);                                                                   \
  }

#define u8dup(u) (uint8_t *)strdup(u)

int main(void) {
  fwbt_t fwbt;
  fwbt_error_t err = fwbt_parse_bytes(test_fwbt, test_fwbt_len, &fwbt);
  CHECK_ERR(err);

  fprintf(stderr, "Header Info:\n");
  fprintf(stderr, "\tSignature: %.*s ; Version: %u\n", 4, fwbt.header.signature,
          fwbt.header.version);
  fprintf(stderr, "\tKey Width: %u ; Value Width: %u ; Entry Count: %u\n",
          fwbt.header.key_width, fwbt.header.value_width,
          fwbt.header.entry_count);

  size_t outsize = 0;
  uint8_t *out;

  err = fwbt_set_value(&fwbt, u8dup("test"), u8dup("abcdefghi"), false);
  CHECK_ERR(err);
  err = fwbt_set_value(&fwbt, u8dup("tes2"), u8dup("abcdefghi"), false);
  CHECK_ERR(err);
  err = fwbt_remove_value(&fwbt, (uint8_t *)"test");
  CHECK_ERR(err);

  err = fwbt_serialize(fwbt, &out, &outsize);
  CHECK_ERR(err);

  fprintf(stderr, "outsize = %zu\n", outsize);
#ifdef WRITE_SERIALIZED
  fwrite(out, outsize, 1, stdout);
#endif

  return err;
}
