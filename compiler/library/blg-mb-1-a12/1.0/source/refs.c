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

uint64_t ref_B0_pr ( void ) { return *(uint64_t*)(0x37d8); }
uint64_t ref_B0_hw ( void ) { return *(uint64_t*)(0x37e0); }
uint64_t ref_B0_fw ( void ) { return *(uint64_t*)(0x37e8); }
uint64_t ref_B0_ch ( void ) { return *(uint64_t*)(0x37f0); }

uint64_t ref_B1_pr ( void ) { return *(uint64_t*)(0x9fe0); }
uint64_t ref_B1_hw ( void ) { return *(uint64_t*)(0x9fe8); }
uint64_t ref_B1_fw ( void ) { return *(uint64_t*)(0x9ff0); }
uint64_t ref_B1_ch ( void ) { return *(uint64_t*)(0x9ff8); }
