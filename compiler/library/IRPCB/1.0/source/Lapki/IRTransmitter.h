#ifndef IRTRANSMITTER_H
#define IRTRANSMITTER_H
#include "IRpkg.h"
#include "Settings.h"

class IRTransmitter {
    inline static int32_t power = *settings.ir_tx_pwr;

   public:
    static void transmit(IRpkg pkg);
    static void setPower(int pwr);
};

#endif