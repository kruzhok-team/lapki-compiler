#pragma once

#include "modem.c"
#include "leds.c"

bool initialized = false;

class NoseBytes {
public:
    int value = 0;
    NoseBytes() {
        if (!initialized) {
            initialized = true;

            ir_modem_init();
            init_modem();
        }
    }

    bool isByteReceived() {
        if (ir_rx_is_available()) {
            value = ir_rx_read_byte();
            initMxLed(mx_a1);
            setMxLed(mx_a1, 1);
            return true;
        }
        return false;
    };

};