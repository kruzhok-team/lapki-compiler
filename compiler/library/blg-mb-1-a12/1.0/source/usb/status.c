#pragma once

//#define SERIAL_LOCATION 0x080037f8U
#define SERIAL_LOCATION 0x080037f8


uint64_t
readSerial_
( void )
{
  return *(uint64_t*)(SERIAL_LOCATION);
}

uint16_t serialStrData[17];

uint16_t *
utfSerial
( void )
{
  uint64_t s = readSerial_();
  char tmp[17] = {0};
  toBase16(tmp,16,s);
  for (int i=0;i<17;i++) {
    // чёрная магия: превращаем ASCII в UTF-8
    serialStrData[i] = (int16_t)tmp[i];
  }
  return serialStrData;
}

struct {
  bool busy;
  bool errProtocol;
  bool failEEPROM;
  bool failAccel;
  bool debug1;
  bool debug2;
  bool debug3;
} status = 
  { .busy = false
  , .errProtocol = false
  , .failEEPROM = false
  , .failAccel = false
  , .debug1 = false
  , .debug2 = false
  , .debug3 = false
  };

void
resetFlags
( void )
{
  status.busy = false;
  status.errProtocol = false;
  status.failEEPROM = false;
  status.failAccel = false;
  status.debug1 = false;
  status.debug2 = false;
  status.debug3 = false;
}

void
flag_errProtocol
( void )
{ status.errProtocol = true; }

void
flag_failEEPROM
( void )
{ status.failEEPROM = true; }

void
flag_failAccel
( void )
{ status.failAccel = true; }
