#ifndef BEAR_IR_TRANSMITTER_CPP
#define BEAR_IR_TRANSMITTER_CPP

#include "BlgIrTransmitter.h"
#include "IRpkg.h"
#include "ir.h"

void BlgIrTransmitter::init() {
    settings.tx_mode = CYBERBEAR_MODE;
}

void BlgIrTransmitter::transmit(IRpkg pkg) {
    lastIrSend = Sys::GetSysTime();
    IRLed::TransmitCyberBearWord(pkg.word16, pkg.bits_count, power, nullptr);
}

// TODO magical numbers: redo with settings v_min v_max
void BlgIrTransmitter::setPower(int pwr) {
    if(pwr < 1) pwr = 1;
    if(pwr > 255) pwr = 255;
    power = pwr;
}

#endif
