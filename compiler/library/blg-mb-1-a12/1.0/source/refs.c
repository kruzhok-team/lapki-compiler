#pragma once

#define _REF(x) uint64_t x;
struct {
  _REF(hardware)
  _REF(firmware)
  _REF(memory)
  _REF(protocol)
  uint8_t me;
} refs =
  { .hardware = 0x5ec3d2c3bff54216
  , .firmware = 0xae2d22d5cacfda77
  , .memory   = 0xd0c862e80d6b672d
  , .protocol = 0x625244e1ecabcfdf
  , .me = 3 //UC
  };

