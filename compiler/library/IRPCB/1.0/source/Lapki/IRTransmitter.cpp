#include "IRTransmitter.h"

#include "IRpkg.h"
#include "ir.h"

void IRTransmitter::transmit(IRpkg pkg) {
    IRLed::TransmitWord(pkg.word16, pkg.bits_count, power, nullptr);
}

// TODO magical numbers: redo with settings v_min v_max
void IRTransmitter::setPower(int pwr) {
    if(pwr < 1) pwr = 1;
    if(pwr > 255) pwr = 255;
    power = pwr;
}