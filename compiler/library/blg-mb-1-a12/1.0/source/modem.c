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
        ir_modem();
        acoustic_modem();
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
