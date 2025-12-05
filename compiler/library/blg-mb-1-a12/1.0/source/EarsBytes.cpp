#pragma once

#include "./commonEars.hpp"
#include "modem.c"

bool initialized = false;

class EarsBytes {
public:
    int value = 0;
    EarsBytes() {
        if (!initialized) {
            if (!detail::microphone::isInit) {
                mrx::hal::microphone::api::init();
                mrx::hal::microphone::detail::initDetector();
                detail::microphone::isInit = true;
            }

            initialized = true;
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
