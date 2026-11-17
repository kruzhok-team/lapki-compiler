#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

struct GPIO_TypeDef {};
struct Registers { uint32_t AHB2ENR = 0; uint32_t DR = 0; };
static GPIO_TypeDef gpio;
static Registers rcc, adc;
static GPIO_TypeDef *GPIOE = &gpio;
static Registers *RCC = &rcc, *ADC1 = &adc;
static const uint32_t RCC_AHB2ENR_GPIOEEN = 1;
static const int ON = 1, OFF = 2;
static int led;
static uint32_t irq_mask;
static uint32_t __get_PRIMASK() { return irq_mask; }
static void __disable_irq() { irq_mask = 1; }
static void __set_PRIMASK(uint32_t mask) { irq_mask = mask; }
static void initPin_PP(GPIO_TypeDef *, int) {}
static void setPin_PP(GPIO_TypeDef *, int, int value) { led = value; }
#include "ir_modem.c"

static void reset() {
    ir_tx = {};
    ir_rx = {};
    ir_modem_init();
}
static void silence(unsigned samples = 2048, float baseline = 2048.0f) {
    for (unsigned i = 0; i < samples; ++i)
        ir_rx_advance(baseline);
}
static std::vector<int> receive(const std::vector<int>& bytes, unsigned seed = 1) {
    reset();
    silence();
    std::vector<int> result;
    for (int byte : bytes) {
        ir_tx_send_byte(byte);
        for (unsigned i = 0; i < 11 * IR_BIT_SAMPLES; ++i) {
            seed = seed * 1664525u + 1013904223u;
            float noise = (static_cast<int>((seed >> 16) & 255) - 128) * 0.2f;
            ir_rx_advance(1000.0f + (ir_tx_advance() == ON ? 200.0f : 0.0f) + noise);
            if (ir_rx_is_available())
                result.push_back(ir_rx_read_byte());
        }
    }
    for (unsigned i = 0; i < 2048; ++i) {
        ir_rx_advance(1000.0f);
        if (ir_rx_is_available())
            result.push_back(ir_rx_read_byte());
    }
    return result;
}

int main(int argc, char **argv) {
    if (argc > 1 && std::string(argv[1]) == "trace") {
        float sample;
        unsigned tick = 0;
        std::cout.precision(9);
        while (std::cin >> sample) {
            ir_rx_advance(sample);
            if (tick % 16 == 0)
                std::cout << tick << ' ' << ir_rx.envelope[0] << ' '
                          << ir_rx.envelope[1] << '\n';
            if (ir_rx_is_available())
                std::cout << "byte " << ir_rx_read_byte() << '\n';
            ++tick;
        }
        return 0;
    }

    reset();
    ir_on();
    assert(led == ON && ir_tx_advance() == ON);
    ir_off();
    assert(led == OFF && ir_tx_advance() == OFF);
    std::vector<int> all;
    for (int i = 0; i < 256; ++i) all.push_back(i);
    auto received = receive(all);
    if (received != all) {
        std::cerr << "received " << received.size() << " bytes\n";
        for (unsigned i = 0; i < received.size(); ++i)
            if (i >= all.size() || received[i] != all[i])
                std::cerr << i << ": " << received[i] << '\n';
    }
    assert(received == all);
    // Check every output sample against the wire format independently of RX.
    for (int byte = 0; byte < 256; ++byte) {
        reset();
        ir_tx_send_byte(byte);
        for (unsigned t = 0; t < 10 * 256; ++t) {
            unsigned symbol = t / 256;
            unsigned period = symbol >= 2 && (byte & (128 >> (symbol - 2))) ? 32 : 4;
            int expected = symbol != 1 && t % period < period / 2 ? ON : OFF;
            assert(ir_tx_advance() == expected);
        }
        for (unsigned t = 0; t < 256; ++t) assert(ir_tx_advance() == OFF);
        assert(!ir_tx.active);
    }

    // An independent sinusoidal sender exercises carrier phase and polarity.
    for (unsigned offset = 0; offset < 32; ++offset) {
        reset();
        silence(2048 + offset, 1800.0f);
        const unsigned byte = 0x96;
        std::vector<int> result;
        for (unsigned t = 0; t < 4096; ++t) {
            unsigned symbol = t / 256;
            unsigned period = symbol >= 2 && symbol < 10 &&
                              (byte & (128 >> (symbol - 2))) ? 32 : 4;
            float carrier = symbol != 1 && symbol < 10 ?
                35.0f * std::sin(6.28318530718f * (t + offset) / period) : 0.0f;
            ir_rx_advance(1800.0f + (offset % 2 ? -carrier : carrier));
            if (ir_rx_is_available()) result.push_back(ir_rx_read_byte());
        }
        assert(result == std::vector<int>{byte});
    }

    // Queue saturation preserves FIFO contents; the 16th new byte is dropped.
    reset();
    silence();
    for (int i = 0; i < 16; ++i) ir_tx_send_byte(i);
    for (unsigned t = 0; t < 16 * 11 * 256; ++t)
        ir_rx_advance(2048.0f + (ir_tx_advance() == ON ? 200.0f : 0.0f));
    for (int i = 0; i < 15; ++i) {
        assert(ir_rx_is_available());
        assert(ir_rx_read_byte() == i);
    }
    assert(!ir_rx_is_available());
    // Fill the receive queue while the consumer is idle, then recover.
    reset();
    silence();
    for (int i = 0; i < 20; ++i) {
        ir_tx_send_byte(i);
        for (unsigned t = 0; t < 11 * 256; ++t)
            ir_rx_advance(2048.0f + (ir_tx_advance() == ON ? 200.0f : 0.0f));
    }
    silence();
    for (int i = 0; i < 15; ++i) assert(ir_rx_read_byte() == i);
    assert(!ir_rx_is_available());
    ir_tx_send_byte(0xa5);
    for (unsigned t = 0; t < 4096; ++t)
        ir_rx_advance(2048.0f + (ir_tx_advance() == ON ? 200.0f : 0.0f));
    assert(ir_rx_is_available() && ir_rx_read_byte() == 0xa5);

    // Mode changes cancel queued/active frames and preserve the caller's mask.
    reset();
    irq_mask = 1;
    ir_tx_send_byte(0xff);
    assert(irq_mask == 1);
    ir_modem_init(); // Another component must not reset a frame or queue.
    assert(ir_tx.head != ir_tx.tail);
    assert(ir_tx_advance() == ON);
    ir_tx_send_byte(0x55);
    ir_off();
    assert(irq_mask == 1 && !ir_tx.active && ir_tx.head == ir_tx.tail);
    for (unsigned t = 0; t < 4096; ++t) assert(ir_tx_advance() == OFF);
    ir_on();
    assert(irq_mask == 1);
    for (unsigned t = 0; t < 4096; ++t) assert(ir_tx_advance() == ON);
    ir_tx_send_byte(0x96);
    assert(irq_mask == 1 && !ir_tx.turn_mode);
    irq_mask = 0;
    ir_off();
    assert(irq_mask == 0);

    // No false byte from DC, deterministic noise, continuous carrier, or header only.
    for (unsigned mode = 0; mode < 4; ++mode) {
        reset();
        silence();
        uint32_t seed = 1;
        for (unsigned t = 0; t < 24000; ++t) {
            seed = seed * 1664525u + 1013904223u;
            float value = 2048.0f;
            if (mode == 1) value += (static_cast<int>(seed >> 24) - 128) * 0.2f;
            if (mode == 2 || (mode == 3 && t < 256))
                value += t % 4 < 2 ? 200.0f : 0.0f;
            ir_rx_advance(value);
        }
        assert(!ir_rx_is_available());
    }
    // IR alone must not feed an uninitialized photodiode ADC into the decoder.
    reset();
    for (unsigned t = 0; t < 1024; ++t) ir_modem();
    assert(ir_rx.margin == 0);
    ir_rx.is_enabled = 1;
    ir_modem();
    assert(ir_rx.margin == 1);
    std::cout << "all IR protocol checks passed\n";
}
