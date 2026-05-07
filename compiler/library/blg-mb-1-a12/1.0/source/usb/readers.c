#pragma once

static
uint64_t
getHex64
( uint8_t pos, uint8_t *buf )
{
  uint8_t tmp[8];
  for (uint8_t i=0; i<8; i++ ) {
    tmp[i] = buf[pos+7-i];
  }
  return *(uint64_t*)(tmp);
}

static
uint32_t
getHex32
( uint8_t pos, uint8_t *buf )
{
  return *(uint32_t*)(buf+pos);
}
