#ifndef IRTRANSMITTER_H
#define IRTRANSMITTER_H
#include "IRpkg.h"
#include "Settings.h"

class IRTransmitter {
    inline static int32_t power = 90;

   public:
    inline static uint32_t lastIrSend = 0;
    static void transmit(IRpkg pkg);
    static void setPower(int pwr);
};

#endif