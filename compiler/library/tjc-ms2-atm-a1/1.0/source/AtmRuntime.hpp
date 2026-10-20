#pragma once
#include <stdint.h>
#include "stm32g030xx.h"

extern "C" uint32_t millis();

namespace atm {
inline bool due(uint32_t now, uint32_t deadline) {
    return int32_t(now - deadline) >= 0;
}
// HSI16, no PLL or external clock dependency.
inline void microDelay(uint32_t us) {
    for (uint32_t i = 0; i < us; ++i)
        for (volatile uint32_t j = 0; j < 4; ++j) __NOP();
}
inline void pinMode(GPIO_TypeDef* port, uint8_t pin, uint32_t mode, uint8_t af = 0) {
    port->BSRR = 1u << pin;
    port->OTYPER |= 1u << pin;
    port->PUPDR &= ~(3u << (pin * 2));
    port->OSPEEDR &= ~(3u << (pin * 2));
    port->AFR[pin / 8] = (port->AFR[pin / 8] & ~(15u << ((pin % 8) * 4))) |
                         (uint32_t(af) << ((pin % 8) * 4));
    port->MODER = (port->MODER & ~(3u << (pin * 2))) | (mode << (pin * 2));
}
}
