#pragma once

#include "modem.c"
#include "leds.c"

bool noseInitialized = false;

class NoseBytes {
public:
    int value = 0;
    NoseBytes() = default;

    void on() {
        if (!noseInitialized) {
            if (!mrx::hal::photoDiode::initialized) {
                mrx::hal::photoDiode::init();
                mrx::hal::photoDiode::start();
                mrx::hal::photoDiode::initialized = true;
            }

            noseInitialized = true;
        }
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        ir_rx_set_enabled(true);
        init_modem();
        __set_PRIMASK(mask);
    }

    void off() {
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        ir_rx_set_enabled(false);
        update_modem_timer();
        __set_PRIMASK(mask);
    }

    bool isByteReceived() {
        if (ir_rx.is_enabled && ir_rx_is_available()) {
            value = ir_rx_read_byte();
            return true;
        }
        return false;
    };

};
