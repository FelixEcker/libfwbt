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

#ifndef FWBT_IGNORE_BODY_SIZE
  const size_t calculated_size =
      (out_fwbt->header.key_width + out_fwbt->header.value_width) *
      out_fwbt->header.entry_count;
  if (data_size != calculated_size) {
    return FWBT_INVALID_BODY_SIZE;
  }
#endif

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

  if (out_fwbt->header.key_width == 0) {
    return FWBT_INVALID_KEY_WIDTH;
  }
  if (out_fwbt->header.value_width == 0) {
    return FWBT_INVALID_VALUE_WIDTH;
  }

  return _parse_body(data + FWBT_HEADER_SIZE, data_size - FWBT_HEADER_SIZE,
                     out_fwbt);
}

fwbt_error_t fwbt_serialize(fwbt_t fwbt, uint8_t **out_bytes,
                            size_t *out_size) {
  if (out_bytes == NULL || out_size == NULL) {
    return FWBT_NULLPTR;
  }

  *out_size = FWBT_HEADER_SIZE;
  *out_size += fwbt.header.element_count *
               (fwbt.header.key_width + fwbt.header.value_width);

  *out_bytes = malloc(*out_size);
  if (*out_bytes == NULL) {
    return FWBT_MALLOC_FAIL;
  }

  memcpy(*out_bytes, &fwbt.header, FWBT_HEADER_SIZE);
  size_t cpy_offs = FWBT_HEADER_SIZE;

  for (size_t ix = 0; ix < fwbt.header.element_count; ix++) {
    memcpy(*out_bytes + cpy_offs, fwbt.body.keys[ix], fwbt.header.key_width);
    cpy_offs += fwbt.header.key_width;
    memcpy(*out_bytes + cpy_offs, fwbt.body.values[ix],
           fwbt.header.value_width);
    cpy_offs += fwbt.header.value_width;
  }

  return FWBT_OK;
}
