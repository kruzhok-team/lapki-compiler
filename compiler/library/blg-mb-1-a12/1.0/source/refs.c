#pragma once

#define _REF(x) uint64_t x;
struct {
  _REF(hardware)
  _REF(firmware)
  _REF(memory)
  _REF(protocol)
  uint8_t me;
} refs =
  { .hardware = 0x08c1d6bb4bf0433f
  , .firmware = 0xb0839d8249cf105e
  , .memory   = 0x9468ee190e322f67
  , .protocol = 0x625244e1ecabcfdf
  , .me = 3 //UC
  };

