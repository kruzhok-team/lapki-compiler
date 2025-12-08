#pragma once

#include "./commonEars.hpp"
#include "modem.c"

bool earsInitialized = false;

class EarsBytes {
public:
    int value = 0;
    EarsBytes() {
        if (!earsInitialized) {
            if (!detail::microphone::isInit) {
                mrx::hal::microphone::api::init();
                mrx::hal::microphone::detail::initDetector();
                detail::microphone::isInit = true;
            }

            earsInitialized = true;
            acoustic_rx.is_enabled = 1;
            init_modem();
        }
    }

    bool isByteReceived() {
        if (acoustic_rx_is_available()) {
            value = acoustic_rx_read_byte();
            return true;
        }
        return false;
    };

};
