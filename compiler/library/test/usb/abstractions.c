#pragma once

#include "buffers.c"

void
insertChar
( uint8_t * target
, uint8_t chr
, short int * pos
) {
  uint8_t index = (*pos);
  target[index] = chr;
  (*pos)++;
}

void
insertSystem_Char
( uint8_t chr )
{ insertChar ( bufSystemA , chr , &lenSystemA); }

void
insertPaddedString
( uint8_t * target
, uint8_t * str
, uint8_t len
, short int * pos
) {
  uint8_t index = (*pos);
  uint8_t i = 0;
  while ( str[i] > 0 && i < len ) {
    target[index] = str[i];
    index++;
    (*pos)++;
    i++;
  }
  for (uint8_t j=i; j<len; j++) {
    target[index] = 0;
    index++;
    (*pos)++;
  }
}

void
insertSystem_PaddedString
( uint8_t * str
, uint8_t len
) { insertPaddedString ( bufSystemA , str , len , &lenSystemA); }

void
insertBool
( uint8_t * target
, bool que
, short int * pos
) {
  char x;
  if (que) x = 't'; else x = 'f';
  insertChar(target,x,pos);
}

void
insertSystem_Bool
( bool que )
{ insertBool ( bufSystemA , que , &lenSystemA); }

void
insertRef
( uint8_t * target
, uint64_t ref
, short int * pos
) {
  uint8_t index = (*pos);
  for (uint8_t i=0; i<8; i++) {
    target[index++] = (uint8_t)( ref >> (8*(7-i)));
    (*pos)++;
  }
}

void
insertSystem_Ref
( uint64_t ref )
{ insertRef ( bufSystemA , ref , &lenSystemA); }
