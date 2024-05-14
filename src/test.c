#include "fwbt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this header file can be generated using the create_test.py script */
#include "../test.fwbt.h"

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
  /* parse the FWBT data */
  fwbt_t fwbt;
  fwbt_error_t err = fwbt_parse_bytes(test_fwbt, test_fwbt_len, &fwbt);
  CHECK_ERR(err);

  fprintf(stderr, "Header Info:\n");
  fprintf(stderr, "\tSignature: %.*s ; Version: %u\n", 4, fwbt.header.signature,
          fwbt.header.version);
  fprintf(stderr, "\tKey Width: %u ; Value Width: %u ; Entry Count: %u\n",
          fwbt.header.key_width, fwbt.header.value_width,
          fwbt.header.entry_count);

  /* Add some new values. Note that in the data which we previously loaded,
     the key width is 4 and value width is 8. So the value will be cut short */
  err = fwbt_set_value(&fwbt, u8dup("test"), u8dup("abcdefghi"), false);
  CHECK_ERR(err);
  err = fwbt_set_value(&fwbt, u8dup("tes2"), u8dup("abcdefghi"), false);
  CHECK_ERR(err);

  /* remove the value with key "test" */
  err = fwbt_remove_value(&fwbt, (uint8_t *)"test");
  CHECK_ERR(err);

  /* serialize our data and send it to stdout */
  size_t outsize = 0;
  uint8_t *out;

  err = fwbt_serialize(fwbt, &out, &outsize);
  CHECK_ERR(err);
#ifdef WRITE_SERIALIZED
  fwrite(out, outsize, 1, stdout);
#endif
  fprintf(stderr, "outsize = %zu\n", outsize);

  return err;
}
