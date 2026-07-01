#ifndef DIGITALIN_H
#define DIGITALIN_H

#include <stdint.h>

class DigitalIn {
    public:
        uint8_t _pin;
        int _oldValue;
        int value;
        DigitalIn(uint8_t pin);
        bool changed();
        bool lowToHigh();
        bool highToLow();

        // loopaction
        void update();
};

#endif