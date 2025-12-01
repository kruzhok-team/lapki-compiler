#include "commonEars.hpp"
#include "leds.c"
extern "C" {
    //
    // Потоковый декодер для передаваемых звуком данных
    // TODO: донастроить в условиях слабого сигнала
    //

    int BIT_INTERVAL = 595;
    int BIT_WINDOW = 250;
    
    // Полосовой фильтр, IIR, ~2.9 КГц
    float filter_2_9_next(float x) {
        // initMxLed(mx_c1);
        // initMxLed(mx_c2);
        // initMxLed(mx_c3);
        // setMxLed(mx_c1, 1);
        static float y_past[3] = {0.0f};
        static float x_past[3] = {0.0f};
        
        // Шкалу сдвинуть
        x -= 2048.0f;
        
        const float feedforw[] = {1.0/40.0, 1.0/80.0, -1.0/27.0};
        const float feedback[] = {1.0, -9.0/5.0, 11.0/12.0};
        
        // setMxLed(mx_c1, 1);
        x_past[2] = x_past[1];
        x_past[1] = x_past[0];
        x_past[0] = x;
        // setMxLed(mx_c2, 1);
        y_past[2] = y_past[1];
        y_past[1] = y_past[0];
        y_past[0] = 	feedforw[0] * x_past[0] +
        feedforw[1] * x_past[1] +
        feedforw[2] * x_past[2] -
        feedback[1] * y_past[1] -
        feedback[2] * y_past[2];
        // setMxLed(mx_c3, 1);
        return y_past[0];
    }

    // Полосовой фильтр, IIR, 6.0 КГц
    float filter_6_0_next(float x) {
        static float y_past[3] = {0.0f};
        static float x_past[3] = {0.0f};
        
        // Шкалу сдвинуть
        x -= 2048.0f;
        
        const float feedforw[] = {1.0/33.0, 1.0/412.0, -1.0/31.0};
        const float feedback[] = {1.0, -11.0/8.0, 14.0/15.0};
        
        x_past[2] = x_past[1];
        x_past[1] = x_past[0];
        x_past[0] = x;
        
        y_past[2] = y_past[1];
        y_past[1] = y_past[0];
        y_past[0] = 	feedforw[0] * x_past[0] +
        feedforw[1] * x_past[1] +
        feedforw[2] * x_past[2] -
        feedback[1] * y_past[1] -
        feedback[2] * y_past[2];
        
        return y_past[0];
    }


    #define ABS(val) ((val) > 0 ? (val) : -(val))

    typedef void (*rx_func_t)(float u, float v);

    static void preamble_detect(float u, float v);
    static void preamble_sync(float u, float v);
    static void link_training(float u, float v);
    static void bit_sampling(float u, float v);

    static struct {
        rx_func_t fn;
        uint32_t tick;
        
        int preamble_ttl;
        int preamble_loc;
        float preamble_max;
        
        int training_ttl;
        float training_max;
        
        float bit_thresh;
        int bit_holdoff;
        int bit_pend;
        int bit_ttl;
        int bit_acc;
        float bit_max;
        
        int have_byte;
        // TODO: чувствительность как параметр
    } rx = { .fn = preamble_detect, 0 };

    static void preamble_detect(float u, float v) {
        if (ABS(v) > 200.0f) {
            rx.preamble_ttl = 100;
            rx.preamble_loc = 100;
            rx.preamble_max = ABS(v);
            rx.fn = preamble_sync;
        }
    }

    static void preamble_sync(float u, float v) {
        if (rx.preamble_ttl >= 0) {
            rx.preamble_ttl--;

            if (ABS(v) > rx.preamble_max) {
                rx.preamble_max = ABS(v);
                rx.preamble_loc = 100 - rx.preamble_ttl;
            }
            return;
        }
        
        if (rx.preamble_loc >= 0) {
            rx.preamble_loc--;
            return;
        }
        
        rx.training_ttl = BIT_WINDOW;
        rx.training_max = 0.0f;
        rx.fn = link_training;
    }

    static void link_training(float u, float v) {
        if (rx.training_ttl >= 0) {
            rx.training_ttl--;
            
            if (ABS(u) > rx.training_max) {
                rx.training_max = ABS(u);
            }
            return;
        }
        
        rx.bit_thresh = 0.75f * rx.training_max;
        rx.bit_pend = 8;
        rx.bit_ttl = BIT_WINDOW;
        rx.bit_holdoff = BIT_INTERVAL - BIT_WINDOW;
        rx.bit_max = 0.0f;
        rx.bit_acc = 0;
        rx.fn = bit_sampling;
    }

    static void bit_sampling(float u, float v) {
        if (rx.bit_holdoff >= 0) {
            rx.bit_holdoff--;
            return;
        }
        
        if (rx.bit_ttl >= 0) {
            rx.bit_ttl--;
            
            if (ABS(u) > rx.bit_max) {
                rx.bit_max = ABS(u);
            }
            return;
        }
        
        int bit_detected = (rx.bit_max >= rx.bit_thresh);
        
        rx.bit_pend--;
        rx.bit_acc += (bit_detected << rx.bit_pend);
        
        if (rx.bit_pend > 0) {
            rx.bit_holdoff = BIT_INTERVAL - BIT_WINDOW;
            rx.bit_ttl = BIT_WINDOW;
            rx.bit_max = 0.0f;
            return;
        }
        
        rx.fn = preamble_detect;
        rx.have_byte = 1;
    }

    int rx_is_available() {
        return rx.have_byte;
    }

    int rx_read_byte() {
        rx.have_byte = 0;
        return rx.bit_acc & 0b11111111;
    }

    // Диспетчеризирующая функция
    // u     2.9 кГц сигнал
    // v     6.0 кГц сигнал
    void rx_advance(float u, float v) {
        // setMxLed(mx_a5, 1);
        if (rx.fn) {
            rx.fn(u, v);
        }
        rx.tick++;
        // setMxLed(mx_a6, 1);
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
    void
    initFrequency
    ( void )
    {
        // initMxLed(mx_b2);
        RCC -> APB2ENR |= RCC_APB2ENR_TIM17EN;
        TIM17 -> CR1 &= ~TIM_CR1_CEN;
        TIM17 -> CNT = 0;
        TIM17 -> ARR = 75 - 1; // 48 kHz
        TIM17 -> PSC = 10 - 1;
        TIM17 -> DIER |= TIM_DIER_UIE;
        TIM17 -> CR1 |= TIM_CR1_CEN;
        
        NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);
        NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn,0);
        // setMxLed(mx_b2, 1);
    }

    uint32_t tim17_up = 0;

    // 0	USB housekeeping
    // 1	System
    // 2	Data
    #define TEST_FIXTURE_DATA_EP 2

    // 32 samples, 64 bytes
    uint16_t ch1_buf[32];

    void
    TIM1_TRG_COM_TIM17_IRQHandler
    ( void )
    {
        // initMxLed(mx_a3);
        // setMxLed(mx_a3, 1);
        // initMxLed(mx_a5);
        // initMxLed(mx_a6);
        // stm32g431::ears::adcArray[0];
        // setMxLed(mx_a3, 1);
        //  ch1_buf[tim17_up % 32] = filter_6_0_next(adcArray[0]) + 2048;
        //  ch1_buf[tim17_up % 32] = stm32g431::ears::adcArray[0];
        //
        // if (tim17_up % 32 == 0)
            // USB_Transmit(2, (char*)ch1_buf, 64);
        
        // initMxLed(mx_a4);

        rx_advance(
            filter_2_9_next(stm32g431::ears::adcArray[0]),
            filter_6_0_next(stm32g431::ears::adcArray[1])
        );

        // setMxLed(mx_a3, 1);
        
        TIM17 -> SR &= ~TIM_SR_UIF;
        tim17_up++;
        // setMxLed(mx_a4, 1);
    }
}
