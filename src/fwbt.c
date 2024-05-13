#include "fwbt.h"

#include <endian.h>
#include <stdlib.h>
#include <string.h>

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
uint32_t _reverse_bytes(uint32_t bytes) {
  uint32_t aux = 0;
  uint8_t byte;
  int i;

  for (i = 0; i < 32; i += 8) {
    byte = (bytes >> i) & 0xff;
    aux |= byte << (32 - 8 - i);
  }
  return aux;
}
#endif

fwbt_error_t _parse_body(const uint8_t *data, size_t data_size,
                         fwbt_t *out_fwbt) {
  if (out_fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  out_fwbt->body.keys =
      malloc(sizeof(*out_fwbt->body.keys) * out_fwbt->header.entry_count);
  out_fwbt->body.values =
      malloc(sizeof(*out_fwbt->body.values) * out_fwbt->header.entry_count);

  const size_t inc = out_fwbt->header.key_width + out_fwbt->header.value_width;
  size_t wix = 0;
  for (size_t rix = 0; rix < data_size; rix += inc) {
    out_fwbt->body.keys[wix] = malloc(out_fwbt->header.key_width);
    out_fwbt->body.values[wix] = malloc(out_fwbt->header.value_width);

    memcpy(out_fwbt->body.keys[wix], data + rix, out_fwbt->header.key_width);
    memcpy(out_fwbt->body.values[wix], data + rix + out_fwbt->header.key_width,
           out_fwbt->header.value_width);

    wix++;
  }

  return FWBT_OK;
}

fwbt_error_t fwbt_parse_bytes(const uint8_t *data, size_t data_size,
                              fwbt_t *out_fwbt) {
  if (out_fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  if (data_size < FWBT_HEADER_SIZE) {
    return FWBT_TOO_SHORT;
  }

  if (data[FWBT_DATA_VERSION_INDEX] != FWBT_VERSION) {
    return FWBT_UNSUPPORTED_VERSION;
  }

  const char signature[] = FWBT_SIGNATURE;
  if (strncmp((const char *)data, signature, sizeof(signature)) != 0) {
    return FWBT_NO_SIGNATURE;
  }

  memcpy(out_fwbt, data, FWBT_HEADER_SIZE);

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
  /* reverse values for little endian systems */
  out_fwbt->header.key_width = _reverse_bytes(out_fwbt->header.key_width);
  out_fwbt->header.value_width = _reverse_bytes(out_fwbt->header.value_width);
  out_fwbt->header.entry_count = _reverse_bytes(out_fwbt->header.entry_count);
#endif

  return _parse_body(data + FWBT_HEADER_SIZE, data_size - FWBT_HEADER_SIZE,
                     out_fwbt);
}
