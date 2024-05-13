# File Structure
# -> Header (17 bytes total)
#   4 bytes : Magic/Signature Bytes = FWBT
#   1 byte  : Version = 1
#   4 bytes : Key Width
#   4 bytes : Value Width
#   4 bytes : Entry Count
# -> Body ( (Key Width + Value Width * Entry Count) bytes total )
#   {
#     [Key Width] bytes   : Key
#     [Value Width] bytes : Value
#   } * Entry Count

import random
import sys

signature_version = [ord('F'), ord('W'), ord('B'), ord('T'), 1]
header_size = 17

def create_body(entry_count, key_width, val_width):
  body = []
  for i in range(0, entry_count):
    for j in range(0, key_width):
      body.append(random.randint(0, 255))

    for j in range(0, val_width):
      body.append(random.randint(0, 255))

  return body

def main():
  key_width = 4
  val_width = 8
  entry_count = 10
  header = (bytearray(signature_version) +
          bytearray(key_width.to_bytes(4, 'big')) +
          bytearray(val_width.to_bytes(4, 'big')) +
          bytearray(entry_count.to_bytes(4, 'big')))
  body = create_body(entry_count, key_width, val_width)
  data = header + bytearray(body)

  expected_length = header_size + (entry_count * (key_width + val_width))
  actual_length = len(header) + len(body)

  print(f"expected total length: {expected_length}", file=sys.stderr)
  print(f"actual total length: {actual_length}", file=sys.stderr)

  assert expected_length == actual_length

  sys.stdout.buffer.write(data)

if __name__ == "__main__":
  main()