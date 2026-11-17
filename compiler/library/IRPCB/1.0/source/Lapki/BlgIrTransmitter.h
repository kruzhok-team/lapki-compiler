#ifndef BEAR_IR_TRANSMITTER
#define BEAR_IR_TRANSMITTER
#ifndef IR_TRANSMITTER_MODE
    #define IR_TRANSMITTER_MODE
#else
    #error "It is not possible to use the CyberBear transmitter and the standard transmitter together!"
#endif


#include "IRpkg.h"
#include "Settings.h"
#include "ir.h"
#include <cstdint>


class BlgIrTransmitter {
    inline static int32_t power = 90;
public:
    inline static uint32_t lastIrSend = 0;

    static void transmit(IRpkg pkg);

    // TODO magical numbers: redo with settings v_min v_max
    static void setPower(int pwr);

    static void init();
};

#endif
