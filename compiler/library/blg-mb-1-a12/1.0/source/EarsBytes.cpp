#pragma once

#include "./acoustic_modem.c"
#include "./commonEars.hpp"

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
            initFrequency();
        }
    }

    bool isByteReceived() {
        if (rx_is_available()) {
            value = rx_read_byte();
            return true;
        }
        return false;
    };

};
