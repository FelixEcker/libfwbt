unit libfwbt;

{ 
  libfwbt -- fixed width binary table library
  
  This unit is a manual translation of the main header file of libfwbt with some
  wrapper functions to make working with libfwbt using fpc a bit more pleasant.
  All functions which are directly linked to the libfwbt functions are prefixed
  with "__extern_"

  See: https://github.com/FelixEcker/libfwbt

  Original File   : include/fwbt.h
  API Version     : 1.0
  FPC API Version : 0.1
  Format Version  : 1
  Author          : Marie Eckert

  Copyright (c) 2024, Marie Eckert
  This Unit is licensed under the BSD 3-Clause license.
}

interface

uses baseunix, ctypes;

const
  sharedobject = 'fwbt';

type
  {$linklib fwbt}

  PUInt8 = ^UInt8;
  PPUInt8 = ^PUInt8;
  Psize_t = ^size_t;

  {$PACKRECORDS 1}
  TFWBTHeader = packed record
    signature  : array[0..3] of CChar;
    version    : UInt8;
    key_width  : UInt32;
    value_width: UInt32;
    entry_count: UInt32;
  end;

  {$PACKRECORDS C}
  TFWBTBody = record
    keys  : PPUInt8;
    values: PPUInt8;
  end;

  TFWBT = record
    header: TFWBTHeader;
    body  : TFWBTBody;
  end;

  PFWBT = ^TFWBT;

  {$PACKRECORDS DEFAULT}

  TFWBTError = (FWBT_OK,FWBT_NO_SIGNATURE,FWBT_UNSUPPORTED_VERSION,
    FWBT_INVALID_KEY_WIDTH,FWBT_INVALID_VALUE_WIDTH,
    FWBT_INVALID_ENTRY_COUNT,FWBT_INVALID_BODY_SIZE,
    FWBT_TOO_SHORT,FWBT_NULLPTR,FWBT_MALLOC_FAIL,
    FWBT_DUPLICATE_KEYS,FWBT_KEY_NOT_FOUND,
    FWBT_OUT_OF_RANGE,FWBT_TABLE_FULL);

{
  @brief Parse the given data into a FWBT.
  @note To avoid memcpying every single key/value pair of the body, define
  FWBT_BODY_NO_MEMCPY whilst compiling. This will cause the parsing code
  to just point into the raw data received by the function.
  When defined MAKE SURE NOT TO FREE THE ORIGINAL DATA
 
  @param data Pointer to the data
  @param data_size Size of the data
  @param out_fwbt Destination pointer for the parsed FWBT
  @return FWBT_OK if parsing succeeds, any other possible error if not
}
function __extern_fwbt_parse_bytes(data:PUInt8; data_size:size_t; out_fwbt:PFWBT):TFWBTError;cdecl;external sharedobject name 'fwbt_parse_bytes';

{
  @brief Serialize the given FWBT into bytes
  @param fwbt The FWBT to be serialized
  @param out_bytes Output location for the serialized bytes
  @param out_size The size of the serialized FWBT in bytes
  @return FWBT_OK if serialization succeeds.
}
function __extern_fwbt_serialize(fwbt:TFWBT; out_bytes:PPUInt8; out_size:Psize_t):TFWBTError;cdecl;external sharedobject name 'fwbt_serialize';

{
  @brief Finds the index of a value in a FWBT
  @param fwbt The FWBT to search through
  @param key The key to search with
  @return The index of the the value, UINT32_MAX if not found
}
function __extern_fwbt_find_value(fwbt:TFWBT; key:PUInt8):UInt32;cdecl;external sharedobject name 'fwbt_find_value';

{
  @brief Set a value within the given FWBT. If successful, the pointers to the
  key and value are to be viewed as being owned by the table now.
  @param fwbt The FWBT to modify
  @param key The key of the new value
  @param value The value
  @param replace_existing Should an existing value with the same key be
  replaced?
  @return FWBT_OK on success
}
function __extern_fwbt_set_value(fwbt:PFWBT; key:PUInt8; value:PUInt8; replace_existing:boolean):TFWBTError;cdecl;external sharedobject name 'fwbt_set_value';

{
  @brief Remove a value within the given FWBT
  @param fwbt The FWBT to modify
  @param key The key of the value to remove
  @return FWBT_OK on success
}
function __extern_fwbt_remove_value(fwbt:PFWBT; key:PUInt8):TFWBTError;cdecl;external sharedobject name 'fwbt_remove_value';

{
  @brief Remove a value within the given FWBT
  @param fwbt The FWBT to modify
  @param index The index of the value to remove
  @return FWBT_OK on success
}
function __extern_fwbt_remove_value_by_index(fwbt:PFWBT; index:UInt32):TFWBTError;cdecl;external sharedobject name 'fwbt_remove_value_by_index';

implementation
end.
