#pragma once

#include "writers.c"
#include "status.c"
#include "readers.c"
#include "../reset.c"

bool
packet
( uint8_t code
) {
  if ( lenSystemQ == 0 ) return false;
  return bufSystemQ[0] == code;
}

void
debug
( uint8_t x )
{
  return;
  for ( int i = 0; i<35; i++ ) {
    uint8_t v = OFF;
    if (i==x) v = ON;
    setMxLed(*mxByRows[i],v);
  }
}

extern volatile bool needToJump;

//uint8_t qwe = 0;

void
processQ
( void )
{
  //setImage(digits2[qwe++%10]); currentImage(11);
  if (packet(erfs.Q.e1)) {
    writeE1();
    return;
  }

  if (packet(erfs.Q.status)) {
    writeStatus();
    return;
  }

  if (packet(erfs.Q.reboot)) {
    reset();
    //Далее недостижимо
    writeStatus();
    return;
  }

  if (packet(erfs.Q.rebootInto)) {
    reset_bootloader();
    writeStatus();
    return;
  }

  if (packet(erfs.Q.clearFlags)) {
    resetFlags();
    writeStatus();
    return;
  }

  flag_errProtocol();
  writeStatus();
}
