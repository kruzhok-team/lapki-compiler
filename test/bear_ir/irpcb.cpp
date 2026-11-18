#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "ir.cpp"

// Compile both production drivers. Only peripheral registers are substituted.
struct GPIO_TypeDef {};
struct RccRegisters { uint32_t AHB2ENR = 0, APB2ENR = 0; };
struct AdcRegisters { uint32_t DR = 0; };
struct TimerRegisters {
    uint32_t CR1 = 0, DIER = 0, PSC = 0, ARR = 0, CNT = 0, EGR = 0, SR = 0;
};
static GPIO_TypeDef gpio;
static GPIO_TypeDef *GPIOE = &gpio;
static RccRegisters rcc;
static RccRegisters *RCC = &rcc;
static AdcRegisters adc;
static AdcRegisters *ADC1 = &adc;
static TimerRegisters timer;
static TimerRegisters *TIM16 = &timer;
static const uint32_t RCC_AHB2ENR_GPIOEEN = 1, RCC_APB2ENR_TIM16EN = 2;
static const uint32_t TIM_CR1_CEN = 1, TIM_CR1_URS = 4;
static const uint32_t TIM_DIER_UIE = 1, TIM_SR_UIF = 1, TIM_EGR_UG = 1;
static const int TIM1_UP_TIM16_IRQn = 25, ON = 1, OFF = 2;
static int led = OFF, priority, enabled_irq;
static void NVIC_ClearPendingIRQ(int) {}
static void NVIC_SetPriority(int, int value) { priority = value; }
static void NVIC_EnableIRQ(int irq) { enabled_irq = irq; }
static void initPin_PP(GPIO_TypeDef *, int) {}
static void setPin_PP(GPIO_TypeDef *, int, int value) { led = value; }
#include "irpcb_modem.c"

static std::vector<std::pair<unsigned, uint16_t>> received;
static void packet(uint8_t bits, uint16_t word) { received.emplace_back(bits, word); }

static void reset() {
    ir_off();
    ir_tx = {};
    ir_rx = {};
    ir_pcb_tx = {};
    timer = {};
    ir_pcb_start = nullptr;
    ir_pcb_stop = nullptr;
    primask = 0;
    led = OFF;
    ir_modem_init();
    ir_pcb_init();
    IRRcvr::SetMode(STANDARD_MODE);
    IRRcvr::callbackI = packet;
    received.clear();
    assert(timer.ARR == 320 && timer.PSC == 0);
    assert(priority == 6 && enabled_irq == TIM1_UP_TIM16_IRQn);
    assert(!(timer.CR1 & TIM_CR1_CEN) && !(timer.DIER & TIM_DIER_UIE));
}

static void half_tick() {
    timer.SR |= TIM_SR_UIF;
    TIM1_UP_TIM16_IRQHandler();
}

// Ideal demodulator model: group carrier pulses separated by long gaps.
// Feed the resulting widths to the actual standard IRPCB capture decoder.
static void demodulate(const std::vector<bool>& samples) {
    bool mark = false;
    unsigned start = 0, last_high = 0;
    for (unsigned i = 0; i < samples.size(); ++i) {
        if (samples[i]) {
            if (!mark) { start = i; mark = true; }
            last_high = i;
        }
        else if (mark && i - last_high >= 3) {
            unsigned ticks = last_high + 2 - start;
            unsigned us = (uint64_t(ticks) * IRPCB_HALF_PERIOD * 1000000 +
                           IRPCB_TIMER_HZ / 2) / IRPCB_TIMER_HZ;
            IRRcvr::ProcessIrqI(2, us);
            mark = false;
        }
    }
    assert(!mark);
    IRRcvr::ProcessIrqI(1, 0); // Inter-bit timeout delivers short packets.
}

static std::vector<bool> standard_frame(uint16_t word, unsigned bits) {
    ir_tx_enqueue(word, bits, IR_TX_IRPCB);
    ir_modem();
    assert(ir_pcb_tx.running && ir_tx.active);
    ir_modem_init(); // Constructing another component cannot cancel this frame.
    ir_pcb_init();
    std::vector<bool> samples;
    unsigned cycles = 0, next_modem = 750;
    while (ir_pcb_tx.running) {
        samples.push_back(led == ON);
        half_tick();
        cycles += IRPCB_HALF_PERIOD;
        if (cycles >= next_modem) {
            int previous = led;
            ir_modem();
            assert(led == previous); // The 48 kHz writer must release PE12.
            next_modem += 750;
        }
    }
    assert(!(timer.CR1 & TIM_CR1_CEN) && !(timer.DIER & TIM_DIER_UIE));
    assert(led == OFF && !ir_tx.active);
    return samples;
}

static void check_standard(uint16_t word, unsigned bits) {
    reset();
    auto samples = standard_frame(word, bits);
    // Exact carrier/envelope parity with the existing IRPCB DAC transmitter.
    IRLed::TransmitWord(word, bits, 90, nullptr);
    auto expected = static_cast<const uint8_t *>(IRLed::dma_tx.memory);
    assert(samples.size() == IRLed::dma_tx.count);
    for (unsigned i = 0; i < samples.size(); ++i) assert(samples[i] == (expected[i] != 0));
    IRLed::dma_tx.callback(nullptr, 2);
    demodulate(samples);
    uint16_t mask = static_cast<uint16_t>(0xffffu << (16 - bits));
    assert((received == std::vector<std::pair<unsigned, uint16_t>>{{bits, uint16_t(word & mask)}}));
}

static void mixed_fifo() {
    reset();
    ir_rx.is_enabled = 1;
    for (unsigned i = 0; i < 2048; ++i) { adc.DR = 2048; ir_modem(); }
    std::vector<uint32_t> expected;
    std::vector<int> bear_expected;
    std::vector<std::pair<unsigned, uint16_t>> pcb_expected;
    for (unsigned i = 0; i < 16; ++i) {
        unsigned protocol = i % 3 == 0 ? IR_TX_CYBERBEAR : IR_TX_IRPCB;
        unsigned bits = i % 2 ? 8 : 16;
        uint16_t word = uint16_t(0x9600 | i);
        if (protocol == IR_TX_CYBERBEAR) {
            ir_tx_send_byte(i);
            bits = 8;
            word = i;
        }
        else ir_tx_enqueue(word, bits, protocol);
        if (i < 15) { // One shared queue of 15 waiting packets; #16 is dropped.
            expected.push_back(word | (bits << 16) | (protocol << 21));
            if (protocol == IR_TX_CYBERBEAR) bear_expected.push_back(i);
            else pcb_expected.emplace_back(bits, bits == 8 ? word & 0xff00 : word);
        }
    }
    std::vector<uint32_t> started;
    std::vector<int> bear_received;
    std::vector<bool> waveform;
    unsigned cycles = 0, next_modem = 750;
    unsigned next_half = std::numeric_limits<unsigned>::max();
    unsigned previous_tail = ir_tx.tail;
    // Schedule the two independent hardware clocks at 36 MHz resolution.
    while (cycles < 36000000) {
        if (next_half < next_modem) {
            cycles = next_half;
            half_tick();
            if (ir_pcb_tx.running) {
                waveform.push_back(led == ON);
                next_half += IRPCB_HALF_PERIOD;
            }
            else {
                demodulate(waveform);
                next_half = std::numeric_limits<unsigned>::max();
            }
        }
        else {
            cycles = next_modem;
            next_modem += 750;
            bool was_running = ir_pcb_tx.running;
            adc.DR = 2048 + (led == ON ? 200 : 0);
            ir_modem();
            if (ir_tx.tail != previous_tail) {
                started.push_back(static_cast<uint32_t>(ir_tx.queue[previous_tail]));
                previous_tail = ir_tx.tail;
            }
            if (!was_running && ir_pcb_tx.running) {
                waveform.assign(1, led == ON);
                next_half = cycles + IRPCB_HALF_PERIOD;
            }
            if (ir_rx_is_available()) bear_received.push_back(ir_rx_read_byte());
        }
    }
    assert(started == expected);
    assert(bear_received == bear_expected);
    assert(received == pcb_expected);
    assert(!ir_tx.active && ir_tx.head == ir_tx.tail && !ir_pcb_tx.running);
}

int main() {
    IRLed::Init();
    IRLed::SetCarrierFreq(56000);
    for (unsigned byte = 0; byte < 256; ++byte) check_standard(byte << 8, 8);
    for (unsigned bits = 1; bits <= 16; ++bits)
        for (uint16_t word : {0, 0xffff, 0x9696, 0x1234}) check_standard(word, bits);
    mixed_fifo();

    reset();
    ir_tx_enqueue(0xffff, 16, IR_TX_IRPCB);
    ir_modem();
    for (unsigned i = 0; i < 100; ++i) half_tick();
    ir_tx_send_byte(0x96);
    primask = 1;
    ir_on();
    assert(primask == 1 && led == ON && !ir_tx.active && !ir_pcb_tx.running);
    assert(ir_tx.head == ir_tx.tail);
    half_tick(); // Stale timer IRQ after cancellation cannot change the LED.
    assert(led == ON);
    for (unsigned i = 0; i < 1024; ++i) ir_modem();
    assert(led == ON);
    ir_off();
    assert(primask == 1 && led == OFF);
    primask = 0;
    ir_tx_enqueue(0x5500, 8, IR_TX_IRPCB);
    for (unsigned i = 0; i <= IR_BIT_SAMPLES; ++i) ir_modem();
    assert(ir_pcb_tx.running);
    ir_off();
    half_tick();
    assert(primask == 0 && led == OFF && !ir_pcb_tx.running);
    std::cout << "A12 -> standard IRPCB: carrier parity, all bytes, words, mixed FIFO, cancel\n";
}
