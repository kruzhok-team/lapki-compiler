#include "stm32g030xx.h"
#include "LEDController.hpp"
#if __has_include("LED.h")
#define USE_LED
#endif
void initTimer()
{
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    TIM14->ARR = 120;
    TIM14->PSC = 9; //  The counter clock frequency CK_CNT is equal to f (CK_PSC) / (PSC[15:0] + 1)
    TIM14->DIER |= TIM_DIER_UIE;
    TIM14->CR1 |= TIM_CR1_CEN;
    NVIC_SetPriority(TIM14_IRQn, 90);
    NVIC_EnableIRQ(TIM14_IRQn);
    // RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    // initPin_PP(GPIOC, 6);
    // setPin_OD(GPIOC, 6, 100);
}

void initPWM(void)
{
#ifdef USE_LED
    LEDController::init();
#endif
    initTimer();
    // Инициализация контроллеров?
}
// Без extern C не линкуется
extern "C"
{
    void TIM14_IRQHandler(void)
    {

        TIM14->CR1 &= ~TIM_CR1_CEN;
        TIM14->SR &= ~TIM_SR_UIF;
#ifdef USE_LED
        // RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
        // initPin_PP(GPIOC, 6);
        // setPin_OD(GPIOC, 6, 100);
        LEDController::update();
#endif
        TIM14->CR1 |= TIM_CR1_CEN;
    }
}
