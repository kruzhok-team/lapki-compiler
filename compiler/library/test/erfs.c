#pragma once


#define _CODE(x) uint8_t x;
#define _ADDR(x) struct { uint8_t addr; uint8_t val; } x;

struct {
  struct {
    _CODE(e1)
    _CODE(status)
    _CODE(jump)
    _CODE(rebootInto)
    _CODE(reboot)
    _CODE(jumpToAddress)
    _CODE(forceTest)
    _CODE(clearFlags)
    _CODE(erasePage)
    _CODE(writeFrame)
    _CODE(readFrame)
    _CODE(setSerial)
    _CODE(setVisual)
  } Q;
  struct {
    _CODE(status)
    _CODE(frame)
    _CODE(e1)
  } A;
  struct {
    _ADDR(loadB1)
  } eeprom;
} erfs =
  { .Q =
    { .e1            = 1 //OK
    , .status        = 2 //OK
    , .jump          = 3 //OK
    , .jumpToAddress = 4 //OK
    , .reboot        = 5 //OK
    , .rebootInto    = 6 //TODO
    , .forceTest     = 7 //TODO
    , .clearFlags    = 8 //OK
    , .erasePage     = 9 //OK
    , .writeFrame    = 10 //OK
    , .readFrame     = 11 //OK
    , .setSerial     = 12 //OK
    , .setVisual     = 13 //OK
    }
  , .A =
    { .status = 1
    , .frame = 2
    , .e1 = 3
    }
  , .eeprom =
    { .loadB1 =
      { .addr = 0xff
      , .val  = 1
      }
    }
  };
