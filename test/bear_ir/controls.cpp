#include <cassert>
#include <cstdint>
#include <iostream>

struct GPIO_TypeDef {};
struct RccRegisters { uint32_t AHB2ENR = 0, APB2ENR = 0; };
struct AdcRegisters { uint32_t DR = 2048; };
struct TimerRegisters {
    uint32_t CR1 = 0, CNT = 0, ARR = 0, PSC = 0, DIER = 0, SR = 0, EGR = 0;
};
static GPIO_TypeDef gpio;
static GPIO_TypeDef *GPIOE = &gpio;
static RccRegisters rcc;
static RccRegisters *RCC = &rcc;
static AdcRegisters adc;
static AdcRegisters *ADC1 = &adc;
static TimerRegisters timer;
static TimerRegisters *TIM17 = &timer;
static const uint32_t RCC_AHB2ENR_GPIOEEN = 1, RCC_APB2ENR_TIM17EN = 2;
static const uint32_t TIM_CR1_CEN = 1, TIM_DIER_UIE = 1, TIM_SR_UIF = 1, TIM_EGR_UG = 1;
static const int TIM1_TRG_COM_TIM17_IRQn = 26, ON = 1, OFF = 2;
static int led = OFF;
static unsigned irq_mask, photo_inits, mic_inits, detector_requests;
static uint32_t __get_PRIMASK() { return irq_mask; }
static void __disable_irq() { irq_mask = 1; }
static void __set_PRIMASK(uint32_t value) { irq_mask = value; }
static void NVIC_EnableIRQ(int) {}
static void NVIC_ClearPendingIRQ(int) {}
static void NVIC_SetPriority(int, int) {}
static void initPin_PP(GPIO_TypeDef *, int) {}
static void setPin_PP(GPIO_TypeDef *, int, int value) { led = value; }
static unsigned long millis() { return 0; }
namespace mrx { namespace hal {
namespace photoDiode {
static bool initialized = false;
static void init() { ++photo_inits; }
static void start() {}
}
namespace microphone {
static uint32_t detectedLevel;
namespace api {
static void init() { ++mic_inits; }
static uint16_t senseLeft() { return 0; }
static uint16_t senseRight() { return 0; }
}
namespace detail {
static void initDetector() { ++detector_requests; }
static void resetDetector() {}
static void enableDetector(bool) {}
}
}
} }
#include "Microphone.hpp"
#include "NoseBytes.cpp"
#include "EarsBytes.cpp"
#include "ir.hpp"

static bool running() { return timer.CR1 & TIM_CR1_CEN; }
static void tick() {
    timer.SR |= TIM_SR_UIF;
    TIM1_TRG_COM_TIM17_IRQHandler();
}
static void finish_tx() {
    unsigned ticks = 0;
    while (running()) {
        tick();
        assert(++ticks < 20000);
    }
    assert(!ir_tx.active && ir_tx.head == ir_tx.tail && led == OFF);
}

int main() {
    NoseBytes nose;
    EarsBytes ears, second_ears;
    assert(!ir_rx.is_enabled && !acoustic_rx.is_enabled && !running());
    assert(photo_inits == 0 && mic_inits == 0 && detector_requests == 0);
    assert(!nose.isByteReceived() && !ears.isByteReceived());
    assert(ears.threshold == 400 && second_ears.threshold == 400);
    ears.setThreshold(700);
    assert(second_ears.threshold == 700);
    rx_advance(0, 600);
    assert(acoustic_rx.fn == preamble_detect);
    second_ears.setThreshold(500);
    rx_advance(0, 600);
    assert(acoustic_rx.fn == preamble_sync);
    ears.setThreshold(-1);
    assert(ears.threshold == 0);
    ears.setThreshold(100000);
    assert(ears.threshold == 65535);
    ears.setThreshold(400);

    IR ir; // Even the ordinary emitter alone must leave the idle timer stopped.
    assert(!running());
    ears.on();
    assert(running() && acoustic_rx.is_enabled && !ir_rx.is_enabled);
    assert(mic_inits == 1 && detector_requests == 0);
    auto audio_tick = acoustic_rx.tick;
    tick();
    assert(acoustic_rx.tick == audio_tick + 1 && ir_rx.phase == 0);
    ears.on();
    assert(acoustic_rx.tick == audio_tick + 1); // Idempotent; no reset.
    Microphone loud_sound; // It still needs its own detector after EarsBytes.
    assert(mic_inits == 1 && detector_requests == 1);

    nose.on();
    assert(running() && ir_rx.is_enabled && photo_inits == 1);
    tick();
    unsigned phase = ir_rx.phase;
    irq_mask = 1;
    nose.on();
    assert(irq_mask == 1 && ir_rx.phase == phase && photo_inits == 1);
    irq_mask = 0;
    irq_mask = 1;
    ears.off();
    assert(irq_mask == 1);
    irq_mask = 0;
    assert(running() && !acoustic_rx.is_enabled && ir_rx.is_enabled);
    assert(!acoustic_rx_is_available() && acoustic_rx.tick == 0);
    tick();
    assert(acoustic_rx.tick == 0 && ir_rx.phase != phase);
    nose.off();
    assert(!running() && !(timer.DIER & TIM_DIER_UIE));
    assert(!ir_rx.is_enabled && ir_rx.phase == 0);
    // Even a stale/manual ISR cannot run either filter while both are disabled.
    for (unsigned i = 0; i < 100; ++i) tick();
    assert(acoustic_rx.tick == 0 && ir_rx.phase == 0);

    nose.on();
    ir_rx.queue[0] = 0x96;
    ir_rx.head = 1;
    assert(nose.isByteReceived() && nose.value == 0x96);
    ir_rx.queue[1] = 0x55;
    ir_rx.head = 2;
    ir_rx.reading = 1;
    nose.off();
    assert(!nose.isByteReceived() && !ir_rx.reading && !ir_rx_is_available());
    nose.on();
    assert(!nose.isByteReceived() && ir_rx.margin == 0);
    nose.off();

    ears.on();
    acoustic_rx.have_byte = 1;
    acoustic_rx.bit_acc = 0xa5;
    assert(ears.isByteReceived() && ears.value == 0xa5);
    acoustic_rx.have_byte = 1;
    acoustic_filter_2_9.x_past[0] = 100;
    ears.off();
    assert(!ears.isByteReceived() && !acoustic_rx.have_byte);
    assert(acoustic_filter_2_9.x_past[0] == 0 && ears.threshold == 400);
    ears.on();
    assert(!ears.isByteReceived() && acoustic_rx.fn == preamble_detect);
    ears.off();

    ir.sendByte(0x96); // TX must wake the clock with both receivers disabled.
    assert(running());
    tick();
    assert(ir_tx.active);
    nose.on();
    nose.off(); // Stopping RX during TX cannot stop transmission.
    assert(running() && ir_tx.active);
    finish_tx();
    assert(!ir_rx.is_enabled && !acoustic_rx.is_enabled);
    ir.sendByte(0x55);
    assert(running());
    finish_tx();
    ears.on();
    ir.sendByte(1);
    tick();
    ir.off(); // Cancelling TX cannot stop the other receiver.
    assert(running() && acoustic_rx.is_enabled && !ir_tx.active);
    ears.off();
    ir.on(); // Constant light needs no 48 kHz interrupt stream.
    assert(!running() && led == ON);
    ir.sendByte(5);
    assert(running());
    finish_tx();
    std::cout << "RX controls: default off, threshold, reset, independent channels, TX clock\n";
}
