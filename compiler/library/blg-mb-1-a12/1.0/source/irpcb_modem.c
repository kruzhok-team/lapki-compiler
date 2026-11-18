#pragma once

#include "ir_modem.c"

#if CYBERBEAR_IRPCB_ENABLED

// TIM17 remains at 48 kHz for CyberBear RX/TX and the acoustic modem.
// TIM16 toggles PE12 only during an IRPCB packet. TIM1 is used by initLoad().
extern "C" {
    enum {
        IRPCB_TIMER_HZ = 36000000,
        IRPCB_CARRIER_HZ = 56000,
        IRPCB_HALF_PERIOD = IRPCB_TIMER_HZ / (2 * IRPCB_CARRIER_HZ),
        IRPCB_HEADER = 0,
        IRPCB_HEADER_GAP,
        IRPCB_DATA,
        IRPCB_BIT_GAP,
        IRPCB_TAIL
    };

    // Match the IRPCB DAC driver's rounding to groups of four half-periods.
    static constexpr unsigned ir_pcb_ticks(unsigned us) {
        return ((us * IRPCB_CARRIER_HZ + 1999999) / 2000000) * 4;
    }

    static struct {
        bool initialized;
        volatile bool running;
        uint16_t remaining;
        uint16_t word;
        unsigned bits;
        unsigned segment;
        bool high;
        bool carrier;
    } ir_pcb_tx = {};

    static void ir_pcb_stop_timer() {
        TIM16->CR1 &= ~TIM_CR1_CEN;
        TIM16->DIER &= ~TIM_DIER_UIE;
        TIM16->SR &= ~TIM_SR_UIF;
        NVIC_ClearPendingIRQ(TIM1_UP_TIM16_IRQn);
        ir_pcb_tx.running = false;
        setPin_PP(GPIOE, 12, OFF);
    }

    static void ir_pcb_start_timer(uint16_t word, unsigned bits) {
        ir_pcb_tx.word = word;
        ir_pcb_tx.bits = bits;
        ir_pcb_tx.segment = IRPCB_HEADER;
        ir_pcb_tx.remaining = ir_pcb_ticks(2400);
        ir_pcb_tx.high = true;
        ir_pcb_tx.carrier = true;
        ir_pcb_tx.running = true;
        TIM16->CNT = 0;
        TIM16->EGR = TIM_EGR_UG;
        TIM16->SR &= ~TIM_SR_UIF;
        NVIC_ClearPendingIRQ(TIM1_UP_TIM16_IRQn);
        setPin_PP(GPIOE, 12, ON);
        TIM16->DIER |= TIM_DIER_UIE;
        TIM16->CR1 |= TIM_CR1_CEN;
    }

    static void ir_pcb_next_segment() {
        switch (ir_pcb_tx.segment) {
        case IRPCB_HEADER:
            ir_pcb_tx.segment = IRPCB_HEADER_GAP;
            ir_pcb_tx.carrier = false;
            ir_pcb_tx.remaining = ir_pcb_ticks(600);
            break;
        case IRPCB_HEADER_GAP:
        case IRPCB_BIT_GAP:
            if (ir_pcb_tx.bits) {
                ir_pcb_tx.segment = IRPCB_DATA;
                ir_pcb_tx.carrier = true;
                ir_pcb_tx.remaining = ir_pcb_ticks(ir_pcb_tx.word & 0x8000 ? 1200 : 600);
            }
            else {
                ir_pcb_tx.segment = IRPCB_TAIL;
                ir_pcb_tx.carrier = false;
                ir_pcb_tx.remaining = ir_pcb_ticks(2400);
            }
            break;
        case IRPCB_DATA:
            ir_pcb_tx.word <<= 1;
            --ir_pcb_tx.bits;
            ir_pcb_tx.segment = IRPCB_BIT_GAP;
            ir_pcb_tx.carrier = false;
            ir_pcb_tx.remaining = ir_pcb_ticks(600);
            break;
        default:
            ir_pcb_stop_timer();
            ir_tx.gap = IR_BIT_SAMPLES;
            ir_tx.active = 0;
            if (ir_update_timer) ir_update_timer();
            break;
        }
    }

    void TIM1_UP_TIM16_IRQHandler(void) {
        if (!(TIM16->SR & TIM_SR_UIF))
            return;
        TIM16->SR &= ~TIM_SR_UIF;
        if (!ir_pcb_tx.running)
            return;
        if (--ir_pcb_tx.remaining == 0)
            ir_pcb_next_segment();
        if (ir_pcb_tx.running) {
            ir_pcb_tx.high = !ir_pcb_tx.high;
            setPin_PP(GPIOE, 12, ir_pcb_tx.carrier && ir_pcb_tx.high ? ON : OFF);
        }
    }

    static void ir_pcb_init() {
        if (ir_pcb_tx.initialized)
            return;
        RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
        TIM16->CR1 = TIM_CR1_URS;
        TIM16->DIER = 0;
        TIM16->PSC = 0;
        TIM16->ARR = IRPCB_HALF_PERIOD - 1;
        TIM16->CNT = 0;
        TIM16->EGR = TIM_EGR_UG;
        TIM16->SR &= ~TIM_SR_UIF;
        NVIC_ClearPendingIRQ(TIM1_UP_TIM16_IRQn);
        // Preempt the 48 kHz filter so it cannot stretch carrier half-periods.
        NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 6);
        NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
        ir_pcb_start = ir_pcb_start_timer;
        ir_pcb_stop = ir_pcb_stop_timer;
        ir_pcb_tx.initialized = true;
    }
}

#endif // CYBERBEAR_IRPCB_ENABLED
