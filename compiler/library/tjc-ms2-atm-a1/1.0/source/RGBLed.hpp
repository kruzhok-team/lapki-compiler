#pragma once
#include "AtmRuntime.hpp"

namespace atm { namespace rgb {
static volatile uint32_t color = 0;
static uint32_t frame = 0;
static uint8_t plane = 0;
inline void apply(uint8_t index) {
    if (!index) frame = color;
    const uint32_t bit = 1u << index;
    uint32_t on = 0;
    if (frame & bit) on |= 1u << 5;
    if (frame & (bit << 8)) on |= 1u << 6;
    if (frame & (bit << 16)) on |= 1u << 7;
    GPIOA->BSRR = ((0xe0u & ~on)) | (on << 16);
}
}}

// Open-drain active-low RGB on PA5/6/7. Eight bit planes reproduce the donor's
// 20 us quantum without reserving TIM14 for the demo's automatic hue sweep.
class RGBLed {
public:
    uint8_t red = 0, green = 0, blue = 0;
    void begin() {
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
        for (uint8_t pin = 5; pin <= 7; ++pin) atm::pinMode(GPIOA, pin, 1);
        RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
        TIM3->CR1 = TIM_CR1_ARPE;
        TIM3->PSC = 15; // 1 MHz at HSI16
        TIM3->ARR = 19;
        TIM3->EGR = TIM_EGR_UG;
        TIM3->ARR = 39; // following plane, buffered
        TIM3->SR = 0;
        TIM3->DIER = TIM_DIER_UIE;
        atm::rgb::plane = 0;
        off();
        atm::rgb::apply(0);
        NVIC_SetPriority(TIM3_IRQn, 0);
        NVIC_ClearPendingIRQ(TIM3_IRQn);
        NVIC_EnableIRQ(TIM3_IRQn);
        TIM3->CR1 |= TIM_CR1_CEN;
    }
    void setColor(uint8_t r, uint8_t g, uint8_t b) {
        red = r; green = g; blue = b;
        atm::rgb::color = uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16);
    }
    void off() { setColor(0, 0, 0); }
};

extern "C" void TIM3_IRQHandler() {
    TIM3->SR = 0;
    const uint8_t plane = (atm::rgb::plane + 1) & 7;
    atm::rgb::plane = plane;
    TIM3->ARR = (20u << ((plane + 1) & 7)) - 1;
    atm::rgb::apply(plane);
}
