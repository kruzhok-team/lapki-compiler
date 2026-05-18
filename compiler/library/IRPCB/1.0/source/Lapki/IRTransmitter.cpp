#ifndef IR_TRANSMITTER
#define IR_TRANSMITTER

#include "IRTransmitter.h"

#include "IRpkg.h"
#include "ir.h"

// extern uint8_t mode;
// extern uint8_t STANDARD_MODE;

void IRTransmitter::init() {
    // mode = STANDARD_MODE;
}

void IRTransmitter::transmit(IRpkg pkg) {
    lastIrSend = Sys::GetSysTime();
    IRLed::TransmitWord(pkg.word16, pkg.bits_count, power, nullptr);
}

// TODO magical numbers: redo with settings v_min v_max
void IRTransmitter::setPower(int pwr) {
    if(pwr < 1) pwr = 1;
    if(pwr > 255) pwr = 255;
    power = pwr;
}
#endif
