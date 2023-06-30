#ifndef BUTTON_H
#define BUTTON_H

class Button {
    private:
        int _pin;
    public:
        Button(int pin);
        bool isJustPressed();
};
#endif