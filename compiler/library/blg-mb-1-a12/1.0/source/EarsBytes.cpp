#pragma once

#include "./acoustic_modem.c"
#include "./commonEars.hpp"

bool initialized = false;

class EarsBytes {
public:
    int value = 0;
    EarsBytes() {
        if (!initialized) {
            // initMxLed(mx_a1);
            mrx::hal::microphone::api::init();
            
            mrx::hal::microphone::detail::initDetector();
            mrx::hal::microphone::detail::enableDetector(true);

            initialized = true;
            initFrequency();
            
            // setMxLed(mx_a1, 1);
        }
            // initialized = true;
        // initMxLed(mx_a2);
        // setMxLed(mx_a2, 1);
    }

    bool isByteReceived() {
        if (rx_is_available()) {
            initMxLed(mx_a2);
            setMxLed(mx_a2, 1);
            value = rx_read_byte();
            USB_Transmit(2, (char*) & (struct {int x[16];}) { value, 0 }, 64);
            return true;
        }
        return false;
    }; 

};
