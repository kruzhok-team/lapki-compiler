#pragma once

#include "acoustic_modem.c"
#include "ir_modem.c"

extern "C" {
    // 0	USB housekeeping
    // 1	System
    // 2	Data
    #define TEST_FIXTURE_DATA_EP 2

    uint32_t tim17_up = 0;

    static struct {
        int initialized;
    } modem_settings = { 0 };
    

    // Главное прерывание модема
    void TIM1_TRG_COM_TIM17_IRQHandler(void) {
        if (ir_rx.is_enabled) {
            uint16_t ir_level = ADC1 -> DR;
            ir_tx_advance();
            ir_rx_advance(
                filter_2730_next(ir_level),
                filter_6000_next(ir_level)
            );
        
        //	ir_rx_buf[tim17_up % 32] = ir_level;
        //	if (tim17_up % 32 == 0)
        //		USB_Transmit(TEST_FIXTURE_DATA_EP, (char*)ir_rx_buf, 64);
        
            // if (ir_rx_is_available())
            //     USB_Transmit(TEST_FIXTURE_DATA_EP, (char*) & (struct {int x[16];}) { ir_rx_read_byte(), 0 }, 64);
        
        }
        
        if (acoustic_rx.is_enabled) {
            // TODO: использовать оба уха
            // Получится какая-то диаграмма направленности
            float value = stm32g431::ears::adcArray[0]; // + stm32g431::ears::adcArray[1];

            // Шкалу сдвинуть
            value -= 2048.0f;

            rx_advance(
                filter_2_9_next(value),
                filter_6_0_next(value)
            );
        }
        TIM17 -> SR &= ~TIM_SR_UIF;
        tim17_up++;
    }

    //
    // Timer clock = 36 MHz
    //
    // FIXME: switch to 72 MHz
    //
    // Recall that:
    // PSC = 0	36 MHz clock
    // PSC = 1	18 MHz clock
    // etc...
    //
    // ARR = 0	No output
    // ARR = 1	18 MHz output
    // etc...
    //
    
    static void init_modem(void) {
        if (!modem_settings.initialized) {
            RCC -> APB2ENR |= RCC_APB2ENR_TIM17EN;
            TIM17 -> CR1 &= ~TIM_CR1_CEN;
            TIM17 -> CNT = 0;
            TIM17 -> ARR = 75 - 1; // 48 kHz
            TIM17 -> PSC = 10 - 1;
            TIM17 -> DIER |= TIM_DIER_UIE;
            TIM17 -> CR1 |= TIM_CR1_CEN;
        
            NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);
            NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, 7);
            modem_settings.initialized = 1;
        }
    }
}
