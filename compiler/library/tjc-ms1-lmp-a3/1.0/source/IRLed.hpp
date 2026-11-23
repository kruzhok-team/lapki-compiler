#pragma once

#include "stm32g030xx.h"

namespace cyberbear_ir {
    // Both boards run the APB timers at 16 MHz (PLL 64 MHz, AHB /4, APB /1).
    // One tick per 12 kHz half-period also gives eight ticks per 1.5 kHz half-period.
    constexpr uint32_t timerClock = 16'000'000;
    constexpr uint32_t tickRate = 24'000;
    constexpr uint16_t ticksPerSymbol = 128;
    constexpr uint8_t queueMask = 15;

    static GPIO_TypeDef *port;
    static uint32_t highMask;
    static uint32_t lowMask;
    static uint8_t queue[queueMask + 1];
    static uint8_t head;
    static uint8_t tail;
    static uint16_t tick;
    static uint8_t output; // LED channel numbers start at 1.
    static void stop()
    {
        TIM16->CR1 = 0;
        TIM16->SR = 0;
        NVIC_ClearPendingIRQ(TIM16_IRQn);
    }

    static void setSteadyOutput(bool on)
    {
        if (!port)
            return;

        const uint32_t primask = __get_PRIMASK();
        __disable_irq();
        stop();
        head = tail = 0;
        port->BSRR = on ? highMask : lowMask;
        __set_PRIMASK(primask);
    }
}

extern "C" void TIM16_IRQHandler()
{
    using namespace cyberbear_ir;

    TIM16->SR = 0;
    const uint16_t frameTick = tick++;
    const uint8_t symbol = frameTick / ticksPerSymbol;
    bool high = false;
    if (symbol == 1) {
        high = (frameTick & 1) == 0;
    } else if (symbol >= 3 && symbol <= 10) {
        const bool bit = (queue[tail] >> (10 - symbol)) & 1;
        high = (frameTick & (bit ? 8 : 1)) == 0;
    }
    port->BSRR = high ? highMask : lowMask;

    // One quiet symbol before the start and two after the last bit.
    if (tick == 13 * ticksPerSymbol) {
        tail = (tail + 1) & queueMask;
        if (head == tail)
            stop();
        else
            tick = ticksPerSymbol; // The trailing silence also separates queued frames.
    }
}

class IRLed {
public:
    static void init(int pin)
    {
        using namespace cyberbear_ir;
        if (pin < 1 || pin > 3)
            return;

        const uint32_t primask = __get_PRIMASK();
        __disable_irq();
        output = pin;
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
        port = GPIOA;
        const uint8_t gpioPin = pin + 4;

        highMask = 1u << gpioPin;
        lowMask = highMask << 16;
        port->BSRR = lowMask;
        const uint32_t modeShift = gpioPin * 2u;
        port->OTYPER &= ~highMask;
        port->PUPDR &= ~(3u << modeShift);
        port->MODER = (port->MODER & ~(3u << modeShift)) | (1u << modeShift);

        RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
        stop();
        TIM16->PSC = 0;
        TIM16->ARR = (timerClock + tickRate / 2) / tickRate - 1;
        TIM16->EGR = TIM_EGR_UG;
        TIM16->SR = 0;
        TIM16->DIER = TIM_DIER_UIE;
        NVIC_SetPriority(TIM16_IRQn, 1);
        NVIC_EnableIRQ(TIM16_IRQn);
        __set_PRIMASK(primask);
    }

    static void on()
    {
        cyberbear_ir::setSteadyOutput(true);
    }

    static void off()
    {
        cyberbear_ir::setSteadyOutput(false);
    }

    static void sendByte(int value)
    {
        using namespace cyberbear_ir;
        if (!port)
            return;

        const uint32_t primask = __get_PRIMASK();
        __disable_irq();
        const uint8_t next = (head + 1) & queueMask;
        if (next != tail) {
            const bool idle = head == tail;
            queue[head] = static_cast<uint8_t>(value);
            head = next;
            if (idle) {
                port->BSRR = lowMask;
                tick = 0;
                TIM16->CNT = 0;
                TIM16->CR1 = TIM_CR1_CEN;
            }
        }
        __set_PRIMASK(primask);
    }
};
