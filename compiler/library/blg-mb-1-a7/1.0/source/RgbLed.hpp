#pragma once

// Обособленная часть hal файла

// Здесь определена функция-прерывание для ШИМ rgb led

#ifdef __cplusplus
extern "C" {
#endif

// interrupt function for TIM17
// void TIM1_TRG_COM_TIM17_IRQHandler(void) {

    // handle rgb leds
//     for (int i(0); i < mrx::hal::rgbLed::rgbLedsCount; i++) {

//         auto& c = mrx::hal::rgbLed::rgbControllers[i];

//         if (c.color != nullptr) {

//             // GPIOD->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + 15));
//             mrx::hal::rgbLed::onPin(i+1);
//         }
//     }

    // TIM17 -> SR &= ~TIM_SR_UIF;  // Снимаем флаг прерывания
// }

#ifdef __cplusplus
}
#endif