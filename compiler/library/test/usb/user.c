#pragma once

#define USER_QUEUE_LEN 10

typedef struct {
  uint8_t len;
  uint8_t buf[64];
  bool ready;
} userMessage;

uint32_t uqPtrSend = 0;
uint32_t uqPtrWrite = 0;

userMessage userQueue[USER_QUEUE_LEN] = {{0,{0},false}};

bool
popUserQueue
( short int *len
, uint8_t *buf
) {
  uint8_t ix = uqPtrSend % USER_QUEUE_LEN;
  if ( !userQueue[ix].ready ) return false;
  *len = userQueue[ix].len;
  for ( uint8_t i=0; i<userQueue[ix].len; i++) {
    buf[i] = userQueue[ix].buf[i];
  }
  userQueue[ix].ready = false;
  uqPtrSend++;
}

bool
pushUserQueue
( uint8_t len
, uint8_t * buf
) {
  uint8_t ix = uqPtrWrite % USER_QUEUE_LEN;
  if ( userQueue[ix].ready ) return false;
  userQueue[ix].len = len;
  for ( uint8_t i=0; i<len; i++) {
    userQueue[ix].buf[i] = buf[i];
  }
  userQueue[ix].ready = true;
  uqPtrWrite++;
  return true;
}

bool
pushString
( char * str
) {
  uint8_t len = 0;
  for (uint8_t i=0; i<64; i++) {
    if ( str[i] == 0 ) break;
  }
  pushUserQueue(len,str);
}
