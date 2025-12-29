#ifndef BUTTON_H
#define BUTTON_H
#include <stdint.h>

#include "gd_lib.h"

constexpr uint32_t debounce_delay_ms = 25;

// example: Button btn(Gpio4);
class Button {
    GPIO_TypeDef *pgpio;
    uint32_t apin;

    bool debouncedPinState;        
    bool lastPinState;             
    uint32_t lastTimePinChanged;  
    bool pressEvent = false;
    bool releaseEvent = false;

    void init();
    inline void setPins(GPIO_TypeDef **pgpio, uint32_t *apin,
                        GPIO_TypeDef *pgpio_val, uint32_t apin_val) {
        *pgpio = pgpio_val;
        *apin = apin_val;
    }

   public:
    // оба в компоненты
    Button();
    Button(GPIO_TypeDef *pgpio_high, uint32_t apin_high);

    bool pressed();

    bool released();

    // need to be insterted in superloop
    void update();
};

#endif  // BUTTON_HPP
