#ifndef IR_TRANSMITTER
#define IR_TRANSMITTER
#ifndef IR_TRANSMITTER_MODE
    #define IR_TRANSMITTER_MODE STANDARD
#else
    #error "Нельзя одновременно использовать стандартный передатчик и для КиберМишки!"
#endif

#include "IRTransmitter.h"

#include "IRpkg.h"
#include "ir.h"

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
