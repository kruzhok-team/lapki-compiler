#pragma once

uint8_t bufSystemQ[64];
uint8_t bufSystemA[64];
short int lenSystemQ;
short int lenSystemA;

void
flushSystemQ
( void )
{
  for (uint8_t i=0;i<64;i++) {
    bufSystemQ[i] = 0;
    lenSystemQ = 0;
  }
}

void
flushSystemA
( void )
{
  for (uint8_t i=0;i<64;i++) {
    bufSystemA[i] = 0;
    lenSystemA = 0;
  }
}

uint8_t bufDataQ[64];
uint8_t bufDataA[64];
short int lenDataQ;
short int lenDataA;

void
flushDataQ
( void )
{
  for (uint8_t i=0;i<64;i++) {
    bufDataQ[i] = 0;
    lenDataQ = 0;
  }
}

void
flushDataA
( void )
{
  for (uint8_t i=0;i<64;i++) {
    bufDataA[i] = 0;
    lenDataA = 0;
  }
}
