#pragma once

#include "abstractions.c"
#include "../erfs.c"
#include "../refs.c"
#include "buffers.c"
#include "status.c"

#define e1_full "blg-mb-1-a12"

void
writeE1
( void )
{
  insertSystem_Char ( erfs.A.e1 );
  insertSystem_PaddedString ( e1_full, 16 );
  insertSystem_PaddedString ( _FIR_GIT_REV, 9 );
}

//uint64_t readSerial ( void );

uint32_t targetAddress;

void
writeStatus
( void )
{
  insertSystem_Char ( erfs.A.status );

  insertSystem_Char ( refs.me );
  insertSystem_Ref  ( readSerial_());
  //insertSystem_Ref  ( targetAddress );

  insertSystem_Ref ( refs.hardware );
  insertSystem_Ref ( refs.firmware );
  insertSystem_Ref ( refs.memory );
  insertSystem_Ref ( refs.protocol );

  insertSystem_Bool ( status.busy );
  insertSystem_Bool ( status.errProtocol );
  insertSystem_Bool ( status.failEEPROM );
  insertSystem_Bool ( status.failAccel );

  insertSystem_Bool ( status.debug1 );
  insertSystem_Bool ( status.debug2 );
  insertSystem_Bool ( status.debug3 );
}

void
writerFrame
( uint8_t iPage
, uint8_t iFrame
) {
  insertSystem_Char ( erfs.A.frame );
  insertSystem_Char ( iPage );
  insertSystem_Char ( iFrame );

  for (uint8_t i=0; i<32; i++) {
    uint32_t addr = 0x8008000 + iPage * 0x800 + iFrame*32 + i;
    uint8_t value = *(uint8_t*)addr;
    insertSystem_Char(value);
  }
}

void
writerRefsB0
( void )
{
  insertSystem_Char ( erfs.A.refsB0 );
  insertSystem_Ref ( ref_B0_hw());
  insertSystem_Ref ( ref_B0_fw());
  insertSystem_Ref ( ref_B0_ch());
  insertSystem_Ref ( ref_B0_pr());
}

void
writerRefsB1
( void )
{
  insertSystem_Char ( erfs.A.refsB1 );
  insertSystem_Ref ( ref_B1_hw());
  insertSystem_Ref ( ref_B1_fw());
  insertSystem_Ref ( ref_B1_ch());
  insertSystem_Ref ( ref_B1_pr());
}

