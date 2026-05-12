#ifndef BEAR_IR_TRANSMITTER_CPP
#define BEAR_IR_TRANSMITTER_CPP

#include "BlgIrTransmitter.h"

void BlgIrTransmitter::transmit(IRpkg pkg) {
    lastIrSend = Sys::GetSysTime();
    IRLed::TransmitWord(pkg.word16, pkg.bits_count, power, nullptr);
}

// TODO magical numbers: redo with settings v_min v_max
void BlgIrTransmitter::setPower(int pwr) {
    if(pwr < 1) pwr = 1;
    if(pwr > 255) pwr = 255;
    power = pwr;
}

#endif
