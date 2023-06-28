#ifndef LED_H
#define LED_H

class Led {
    private:
        int _pin;
    public:
        Led(int pin);
        void turnOn();
        void turnOff();
};
#endif