#pragma once

#include "modem.c"
#include "leds.c"

bool initialized = false;

class NoseBytes {
public:
    int value = 0;
    NoseBytes() {
        if (!initialized) {
            if (!mrx::hal::photoDiode::initialized) {
                mrx::hal::photoDiode::init();
                mrx::hal::photoDiode::start();
                mrx::hal::photoDiode::initialized = true;
            }

            ir_modem_init();
            init_modem();

            initialized = true;
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
