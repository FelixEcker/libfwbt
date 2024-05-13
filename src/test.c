#include "fwbt.h"

#include <stdio.h>

#include "../test.fwbt.h"


int main(void) {
  fwbt_t fwbt;
  fwbt_error_t err = fwbt_parse_bytes(test_fwbt, test_fwbt_len, &fwbt);

  printf("Header Info:\n");
  printf("\tSignature: %.*s ; Version: %u\n", 4, fwbt.header.signature,
         fwbt.header.version);
  printf("\tKey Width: %u ; Value Width: %u ; Entry Count: %u\n",
         fwbt.header.key_width, fwbt.header.value_width,
         fwbt.header.entry_count);

  getc(stdin);
  return err;
}
