#ifndef BUTTON_H
#define BUTTON_H
#include <stdint.h>

#include "gd_lib.h"
// example: Button btn(Gpio4);
class Button {
    GPIO_TypeDef *pgpio;
    uint32_t apin;
    inline void init() { Gpio::SetHi(pgpio, apin); }
    // TODO why not references?
    inline void setPins(GPIO_TypeDef **pgpio, uint32_t *apin,
                        GPIO_TypeDef *pgpio_val, uint32_t apin_val) {
        *pgpio = pgpio_val;
        *apin = apin_val;
    }

   public:
    // оба в компоненты
    Button();
    Button(GPIO_TypeDef *pgpio_high, uint32_t apin_high);

    bool isPressed();

    bool isUnpressed();
};

#endif  // BUTTON_HPP
