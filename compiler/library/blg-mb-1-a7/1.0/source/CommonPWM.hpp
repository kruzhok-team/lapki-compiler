#pragma once

// Обособленная часть hal файла

// Здесь определена функция-прерывание для ШИМ

#ifdef __cplusplus
extern "C" {
#endif

volatile uint32_t pwmCounter = 0;

// interrupt function for TIM2
void TIM2_IRQHandler(void) {

    TIM2 -> SR &= ~TIM_SR_UIF;  // Снимаем флаг прерывания

    ++pwmCounter;
    
    // TIM2 -> CR1 &= ~TIM_CR1_CEN;    // Счет запрещён

    // leds
    if (mrx::hal::pwm::interruptFunc != nullptr)
        mrx::hal::pwm::interruptFunc();

    // rgb leds
    mrx::hal::rgbLed::interruptFunc();
    
    // Speaker
    // Speaker has a unique frequency
    // if ((++mrx::hal::speaker::currLevel) >= mrx::hal::speaker::level) {

    mrx::hal::speaker::interruptFunc();
        // mrx::hal::speaker::currLevel = 0;
    // }

    // TIM2 -> CR1 |= TIM_CR1_CEN;  // Счет разрешен
}

#ifdef __cplusplus
}
#endif