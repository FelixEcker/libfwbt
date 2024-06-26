/* FWBT -- Fixed Width Binary Table
 * Format Version 1 , API Version 1.0
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause license.
 */

#include "fwbt.h"

#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <machine/endian.h>
#define __BYTE_ORDER BYTE_ORDER
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#else
#include <endian.h>
#endif

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
#define __USE_LE
#endif

/* NOTE: These two macros should be used in every case where a failed
 * (re)allocation should return a FWBT_MALLOC_FAIL error
 */
#define xmalloc(s)                                                             \
  ({                                                                           \
    void *ret = malloc(s);                                                     \
    if (ret == NULL) {                                                         \
      return FWBT_MALLOC_FAIL;                                                 \
    };                                                                         \
    ret;                                                                       \
  })

#define xrealloc(o, s)                                                         \
  ({                                                                           \
    void *ret = realloc(o, s);                                                 \
    if (ret == NULL) {                                                         \
      return FWBT_MALLOC_FAIL;                                                 \
    }                                                                          \
    ret;                                                                       \
  })

#ifdef __USE_LE
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
  if (data == NULL || out_fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  const uint32_t key_width = out_fwbt->header.key_width;
  const uint32_t value_width = out_fwbt->header.value_width;

#ifndef FWBT_IGNORE_BODY_SIZE
  const size_t calculated_size =
      (key_width + value_width) * out_fwbt->header.entry_count;
  if (data_size != calculated_size) {
    return FWBT_INVALID_BODY_SIZE;
  }
#endif

  out_fwbt->body.keys =
      xmalloc(sizeof(*out_fwbt->body.keys) * out_fwbt->header.entry_count);
  out_fwbt->body.values =
      xmalloc(sizeof(*out_fwbt->body.values) * out_fwbt->header.entry_count);

  const size_t inc = key_width + value_width;
  size_t wix = 0;
  for (size_t rix = 0; rix < data_size; rix += inc) {
#ifndef FWBT_BODY_NO_MEMCPY
    out_fwbt->body.keys[wix] = xmalloc(key_width);
    out_fwbt->body.values[wix] = xmalloc(value_width);
    memcpy(out_fwbt->body.keys[wix], data + rix, key_width);
    memcpy(out_fwbt->body.values[wix], data + rix + key_width, value_width);
#else
    out_fwbt->body.keys[wix] = data + rix;
    out_fwbt->body.values[wix] = data + key_width;
#endif

    wix++;
  }

  return FWBT_OK;
}

fwbt_error_t fwbt_parse_bytes(const uint8_t *data, size_t data_size,
                              fwbt_t *out_fwbt) {
  if (data == NULL || out_fwbt == NULL) {
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

#ifdef __USE_LE
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
  if (out_fwbt->header.entry_count == UINT32_MAX) {
    return FWBT_INVALID_ENTRY_COUNT;
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
  *out_size += fwbt.header.entry_count *
               (fwbt.header.key_width + fwbt.header.value_width);

  *out_bytes = xmalloc(*out_size);

  /* save these in case of byte reversage */
  uint32_t entry_count = fwbt.header.entry_count;
  uint32_t key_width = fwbt.header.key_width;
  uint32_t value_width = fwbt.header.value_width;

#ifdef __USE_LE
  /* reverse values for little endian systems */
  fwbt.header.key_width = _reverse_bytes(fwbt.header.key_width);
  fwbt.header.value_width = _reverse_bytes(fwbt.header.value_width);
  fwbt.header.entry_count = _reverse_bytes(fwbt.header.entry_count);
#endif

  memcpy(*out_bytes, &fwbt.header, FWBT_HEADER_SIZE);
  size_t cpy_offs = FWBT_HEADER_SIZE;

  for (size_t ix = 0; ix < entry_count; ix++) {
    memcpy(*out_bytes + cpy_offs, fwbt.body.keys[ix], key_width);
    cpy_offs += key_width;
    memcpy(*out_bytes + cpy_offs, fwbt.body.values[ix], value_width);
    cpy_offs += value_width;
  }

  return FWBT_OK;
}

uint32_t fwbt_find_value(fwbt_t fwbt, uint8_t *key) {
  if (key == NULL) {
    return UINT32_MAX;
  }

  for (size_t ix = 0; ix < fwbt.header.entry_count; ix++) {
    if (memcmp(fwbt.body.keys[ix], key, fwbt.header.key_width) == 0) {
      return ix;
    }
  }

  return UINT32_MAX;
}

fwbt_error_t fwbt_set_value(fwbt_t *fwbt, uint8_t *key, uint8_t *value,
                            bool replace_existing) {
  if (fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  uint32_t previous = fwbt_find_value(*fwbt, key);
  if (previous != UINT32_MAX) {
    if (!replace_existing) {
      return FWBT_DUPLICATE_KEYS;
    }

    free(fwbt->body.keys[previous]);
    free(fwbt->body.values[previous]);
    fwbt->body.keys[previous] = key;
    fwbt->body.values[previous] = value;
    return FWBT_OK;
  }

  if (fwbt->header.entry_count + 1 == UINT32_MAX) {
    return FWBT_TABLE_FULL;
  }

  previous = fwbt->header.entry_count;
  fwbt->header.entry_count++;
  fwbt->body.keys = xrealloc(fwbt->body.keys, sizeof(*fwbt->body.keys) *
                                                  fwbt->header.entry_count);
  fwbt->body.values = xrealloc(fwbt->body.values, sizeof(*fwbt->body.values) *
                                                      fwbt->header.entry_count);

  fwbt->body.keys[previous] = key;
  fwbt->body.values[previous] = value;
  return FWBT_OK;
}

fwbt_error_t fwbt_remove_value(fwbt_t *fwbt, uint8_t *key) {
  if (fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  uint32_t ix = fwbt_find_value(*fwbt, key);
  if (ix == UINT32_MAX) {
    return FWBT_KEY_NOT_FOUND;
  }

  return fwbt_remove_value_by_index(fwbt, ix);
}

fwbt_error_t fwbt_remove_value_by_index(fwbt_t *fwbt, uint32_t index) {
  if (fwbt == NULL) {
    return FWBT_NULLPTR;
  }

  if (index > fwbt->header.entry_count) {
    return FWBT_OUT_OF_RANGE;
  }

  fwbt->header.entry_count--;
  for (size_t ix = index; ix < fwbt->header.entry_count; ix++) {
    fwbt->body.keys[ix] = fwbt->body.keys[ix + 1];
    fwbt->body.values[ix] = fwbt->body.values[ix + 1];
  }

  return FWBT_OK;
}
