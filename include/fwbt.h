/* FWBT -- Fixed Width Binary Table
 * Format Version 1 , API Version 1.0
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensed under the BSD 3-Clause license.
 */

/*
 * File Structure
 * -> Header (17 bytes total)
 *   4 bytes : Magic/Signature Bytes = FWBT
 *   1 byte  : Version = 1
 *   4 bytes : Key Width (Big Endian)
 *   4 bytes : Value Width (Big Endian)
 *   4 bytes : Entry Count (Big Endian)
 * -> Body ( (Key Width + Value Width * Entry Count) bytes total )
 *   {
 *     [Key Width] bytes   : Key
 *     [Value Width] bytes : Value
 *   } * Entry Count
 */

#ifndef FWBT_H
#define FWBT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cpluplus
extern "C" {
#endif

#define FWBT_SIGNATURE                                                         \
  { 'F', 'W', 'B', 'T' }
#define FWBT_VERSION 1
#define FWBT_API_VERSION "1.0"

typedef struct fwbt_header {
  char signature[4];
  uint8_t version;
  uint32_t key_width;
  uint32_t value_width;
  uint32_t entry_count;
} __attribute__((packed, aligned(1))) fwbt_header_t;

/* clang-format off */

#define FWBT_DATA_VERSION_INDEX       4
#define FWBT_DATA_KEY_WIDTH_INDEX     FWBT_DATA_VERSION_INDEX     + 1
#define FWBT_DATA_VALUE_WIDTH_INDEX   FWBT_DATA_KEY_WIDTH_INDEX   + 4
#define FWBT_DATA_ENTRY_COUNT_INDEX   FWBT_DATA_VALUE_WIDTH_INDEX + 4
#define FWBT_DATA_BODY_START          FWBT_DATA_ETNRY_COUNT_INDEX + 1

/* clang-format on */

#define FWBT_HEADER_SIZE sizeof(fwbt_header_t)

typedef struct fwbt_body {
  uint8_t **keys;
  uint8_t **values;
} fwbt_body_t;

typedef struct fwbt {
  fwbt_header_t header;
  fwbt_body_t body;
} fwbt_t;

typedef enum fwbt_error {
  FWBT_OK,

  /** @brief signature is missing or malformed */
  FWBT_NO_SIGNATURE,

  /** @brief data version does not equal FWBT_VERSION */
  FWBT_UNSUPPORTED_VERSION,

  /** @brief key width is 0 */
  FWBT_INVALID_KEY_WIDTH,

  /** @brief value width is 0 */
  FWBT_INVALID_VALUE_WIDTH,

  /** @brief entry count equals UINT32_MAX */
  FWBT_INVALID_ENTRY_COUNT,

  /** @brief the actual size of the body does not equal
   *         entry_count * (key_width + value_width)
   */
  FWBT_INVALID_BODY_SIZE,

  /** @brief data size is shorter than FWBT_HEADER_SIZE */
  FWBT_TOO_SHORT,

  /** @brief a function received NULL where a valid pointer was expected */
  FWBT_NULLPTR,

  /** @brief an internal memory allocation failed */
  FWBT_MALLOC_FAIL,

  /** @brief a value with the same key already exists */
  FWBT_DUPLICATE_KEYS,

  /** @brief no value with the given key could be found */
  FWBT_KEY_NOT_FOUND,

  /** @brief the given index is greater than or equal to the entry_count */
  FWBT_OUT_OF_RANGE,

  /** @brief the entry_count has reached UINT32_MAX - 1 */
  FWBT_TABLE_FULL
} fwbt_error_t;

/**
 * @brief Parse the given data into a FWBT
 * @param data Pointer to the data
 * @param data_size Size of the data
 * @param out_fwbt Destination pointer for the parsed FWBT
 * @return FWBT_OK if parsing succeeds, any other possible error if not
 */
fwbt_error_t fwbt_parse_bytes(const uint8_t *data, size_t data_size,
                              fwbt_t *out_fwbt);

/**
 * @brief Serialize the given FWBT into bytes
 * @param fwbt The FWBT to be serialized
 * @param out_bytes Output location for the serialized bytes
 * @param out_size The size of the serialized FWBT in bytes
 * @return FWBT_OK if serialization succeeds.
 */
fwbt_error_t fwbt_serialize(fwbt_t fwbt, uint8_t **out_bytes, size_t *out_size);

/**
 * @brief Finds the index of a value in a FWBT
 * @param fwbt The FWBT to search through
 * @param key The key to search with
 * @return The index of the the value, UINT32_MAX if not found
 */
uint32_t fwbt_find_value(fwbt_t fwbt, uint8_t *key);

/**
 * @brief Set a value within the given FWBT. If successful, the pointers to the
 * key and value are to be viewed as being owned by the table now.
 * @param fwbt The FWBT to modify
 * @param key The key of the new value
 * @param value The value
 * @param replace_existing Should an existing value with the same key be
 * replaced?
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_set_value(fwbt_t *fwbt, uint8_t *key, uint8_t *value,
                            bool replace_existing);

/**
 * @brief Remove a value within the given FWBT
 * @param fwbt The FWBT to modify
 * @param key The key of the value to remove
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_remove_value(fwbt_t *fwbt, uint8_t *key);

/**
 * @brief Remove a value within the given FWBT
 * @param fwbt The FWBT to modify
 * @param index The index of the value to remove
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_remove_value_by_index(fwbt_t *fwbt, uint32_t index);

#ifdef __cpluplus
}
#endif
#endif
