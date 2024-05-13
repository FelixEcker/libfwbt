# libfwbt
fixed width binary table library.

## Introduction
The fbtw format is a simple file format intially created to store a large table of
corresponding SHA-256 checksums. To make it usable for different things, the
width of the key column and value column is adjustable per file.

### File Structure
#### Header
* 4 bytes / char[4]: Magic/Signature Bytes = FWBT
* 1 byte / u8: Version = 1
* 4 bytes / u32: Key Width (Big Endian)
* 4 bytes / u32: Value Width (Big Endian)
* 4 bytes / u32: Entry Count (Big Endian)

#### Body
The following elements are repeated for each entry. The number of entries is
specified in the header.
* [Key Width] bytes: Key
* [Value Width] bytes: Value

### Library Usage Example
```c
/* file: src/test.c */

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
#define CHECK_ERR(x) \
  if (x != FWBT_OK) { \
    fprintf(stderr, STR(__LINE__) ": error %d\n", x); \
    exit(x); \
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

  fwrite(out, outsize, 1, stdout);
  fprintf(stderr, "outsize = %zu\n", outsize);

  return err;
}
```