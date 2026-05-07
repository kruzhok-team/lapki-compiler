#pragma once

#include "modem.c"
#include "leds.c"

bool noseInitialized = false;

class NoseBytes {
public:
    int value = 0;
    NoseBytes() {
        if (!noseInitialized) {
            if (!mrx::hal::photoDiode::initialized) {
                mrx::hal::photoDiode::init();
                mrx::hal::photoDiode::start();
                mrx::hal::photoDiode::initialized = true;
            }

            ir_modem_init();
            init_modem();

            noseInitialized = true;
        }
    }

    bool isByteReceived() {
        if (ir_rx_is_available()) {
            value = ir_rx_read_byte();
            return true;
        }
        return false;
    };

};
