/* FWBT -- Fixed Width Binary Table
 * Format Version 1 , API Version 1.0
 *
 * File Structure
 * -> Header (17 bytes total)
 *   4 bytes : Magic/Signature Bytes = FWBT
 *   1 byte  : Version = 1
 *   4 bytes : Key Width
 *   4 bytes : Value Width
 *   4 bytes : Entry Count
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

#define FWBT_SIGNATURE {'F', 'W', 'B', 'T'}
#define FWBT_VERSION 1
#define FWBT_API_VERSION "1.0"

typedef struct fwbt_header {
    char signature[4];
    uint8_t version;
    uint32_t key_width;
    uint32_t value_width;
    uint32_t entry_count;
}__attribute__((packed, aligned(1))) fwbt_header_t;

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
    FWBT_NO_SIGNATURE,
    FWBT_UNSUPPORTED_VERSION,
    FWBT_INVALID_KEY_WIDTH,
    FWBT_INVALID_VALUE_WIDTH,
    FWBT_INVALID_BODY_SIZE
} fwbt_error_t;

/**
 * @brief Parse the given data into a FWBT
 * @param data Pointer to the data
 * @param data_size Size of the data
 * @param out_fwbt Destination pointer for the parsed FWBT
 * @return FWBT_OK if parsing succeeds, any other possible error if not
 */
fwbt_error_t fwbt_parse_bytes(uint8_t *data, size_t data_size, fwbt_t *out_fwbt);

/**
 * @brief Serialize the given FWBT into bytes
 * @param fwbt The FWBT to be serialized
 * @param out_bytes Output location for the serialized bytes
 * @param out_size The size of the serialized FWBT in bytes
 * @return FWBT_OK if serialization succeeds.
 */
fwbt_error_t fwbt_serialize(fwbt_t fwbt, uint8_t **out_bytes, size_t **out_size);

/**
 * @brief Finds the index of a value in a FWBT
 * @param fwbt The FWBT to search through
 * @param key The key to search with
 * @return The index of the the value, UINT32_MAX if not found
 */
uint32_t fwbt_find_value(fwbt_t fwbt, uint8_t *key);

/**
 * @brief Set a value within the given FWBT
 * @param fwbt The FWBT to modify
 * @param key The key of the new value
 * @param value The value
 * @param replace_existing Should an existing value with the same key be replaced?
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_set_value(fwbt_t fwbt, uint8_t *key, uint8_t *value, bool replace_existing);

/**
 * @brief Remove a value within the given FWBT
 * @param fwbt The FWBT to modify
 * @param key The key of the value to remove
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_remove_value(fwbt_t fwbt, uint8_t *key);

/**
 * @brief Remove a value within the given FWBT
 * @param fwbt The FWBT to modify
 * @param index The index of the value to remove
 * @return FWBT_OK on success
 */
fwbt_error_t fwbt_remove_value_by_index(fwbt_t fwbt, uint32_t index);

#endif