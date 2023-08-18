#ifndef DIGITALOUT_H
#define DIGITALOUT_H

class DigitalOut {
    public:
        DigitalOut(uint8_t _pin);
        void high();
        void low();
        void init();
        void setPWM(int val);
        
        uint8_t pin;
};

#endif