#include "Button.h"

#include "gd_lib.h"
#include "board.h"
// оба в компоненты
Button::Button() {
    setPins(&pgpio, &apin, Gpio4);
    init();
}
Button::Button(GPIO_TypeDef *pgpio_high, uint32_t apin_high)
    : pgpio(pgpio_high), apin(apin_high) {
    init();
}

bool Button::isPressed() { return Gpio::IsLo(pgpio, apin); }

bool Button::isUnpressed(){ return !isPressed();}
