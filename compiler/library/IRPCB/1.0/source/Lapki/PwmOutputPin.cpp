#include "PwmOutputPin.h"

#include "core_cm4.h"
#include "gd32e11x_kl.h"
#include "gd_lib.h"

constexpr uint32_t top_value = 255;

// system reset if pins are not PA6 PA7 PB8 PB9 PB0 PB10
PwmSetup getPwmSetup(GPIO_TypeDef* pgpio, uint16_t apin) {
    if (pgpio == GPIOA) {
        if (apin == 6)
            return PwmSetup(pgpio, apin, TIM2, 0, invNotInverted,
                            Gpio::PushPull, top_value);
        if (apin == 7)
            return PwmSetup(pgpio, apin, TIM2, 1, invNotInverted,
                            Gpio::PushPull, top_value);
    } else if (pgpio == GPIOB) {
        if (apin == 8)
            return PwmSetup(pgpio, apin, TIM3, 2, invNotInverted,
                            Gpio::PushPull, top_value);
        if (apin == 9)
            return PwmSetup(pgpio, apin, TIM3, 3, invNotInverted,
                            Gpio::PushPull, top_value);
        if (apin == 0)
            return PwmSetup(pgpio, apin, TIM2, 2, invNotInverted,
                            Gpio::PushPull, top_value);
        if (apin == 1)
            return PwmSetup(pgpio, apin, TIM2, 3, invNotInverted,
                            Gpio::PushPull, top_value);
    }
    NVIC_SystemReset();
    return PwmSetup(pgpio, apin, TIM2, 3, invNotInverted, Gpio::PushPull,
                    top_value);
}

PwmOutputPin::PwmOutputPin(GPIO_TypeDef* pgpio, uint16_t apin)
    : pin(getPwmSetup(pgpio, apin)) {
    pin.Init();
    setDuty(0);
}

void PwmOutputPin::setDuty(uint32_t val) {
    if (val > top_value) val = top_value;
    if (val < 0) val = 0;
    pin.Set(val);
}

void PwmOutputPin::setDutyPortion(float portion) {
    setDuty(static_cast<uint32_t>(portion * top_value));
}
