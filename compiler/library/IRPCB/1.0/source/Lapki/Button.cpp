#include "Button.h"

#include "board.h"
#include "gd_lib.h"
#include "yartos.h"

void Button::init() {
    debouncedPinState = Gpio::IsLo(pgpio, apin);
    lastPinState = debouncedPinState;
    lastTimePinChanged = Sys::GetSysTime();
}

Button::Button() {
    setPins(&pgpio, &apin, Gpio4);
    init();
}
Button::Button(GPIO_TypeDef *pgpio_high, uint32_t apin_high)
    : pgpio(pgpio_high), apin(apin_high) {
    init();
}

void Button::update() {
    bool currentPinState = Gpio::IsLo(pgpio, apin);
    if (currentPinState != lastPinState) lastTimePinChanged = Sys::GetSysTime();
    lastPinState = currentPinState;
    if (Sys::GetSysTime() - lastTimePinChanged >= TIME_MS2I(debounce_delay_ms)) {
        if (currentPinState != debouncedPinState) {
            debouncedPinState = currentPinState;
            if (debouncedPinState == true) {
                pressEvent = true;
            } else {
                releaseEvent = true;
            }
        }
    }
}

bool Button::pressed() {
    bool tmp = pressEvent;
    pressEvent = 0;
    return tmp;
}

bool Button::released() {
    bool tmp = releaseEvent;
    releaseEvent = 0;
    return tmp;
}
