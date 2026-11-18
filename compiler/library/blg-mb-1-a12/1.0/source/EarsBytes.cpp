#pragma once

#include "./commonEars.hpp"
#include "modem.c"

bool earsInitialized = false;

class EarsBytes {
public:
    int value = 0;
    static volatile uint16_t &threshold;

    EarsBytes() = default;

    void setThreshold(int level) {
        if (level < 0) level = 0;
        if (level > 65535) level = 65535;
        threshold = static_cast<uint16_t>(level);
    }

    void on() {
        if (!earsInitialized) {
            if (!detail::microphone::isInit) {
                mrx::hal::microphone::api::init();
                detail::microphone::isInit = true;
            }

            earsInitialized = true;
        }
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        acoustic_rx_set_enabled(true);
        init_modem();
        __set_PRIMASK(mask);
    }

    void off() {
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        acoustic_rx_set_enabled(false);
        update_modem_timer();
        __set_PRIMASK(mask);
    }

    bool isByteReceived() {
        if (acoustic_rx.is_enabled && acoustic_rx_is_available()) {
            value = acoustic_rx_read_byte();
            return true;
        }
        return false;
    };

};

volatile uint16_t &EarsBytes::threshold = acoustic_rx_start_threshold;
