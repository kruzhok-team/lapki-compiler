#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include "CyberBearReceiver.h"

// Include the actual A12 transmitter to check both ends of the wire protocol.
struct GPIO_TypeDef {};
struct Registers { uint32_t AHB2ENR = 0; uint32_t DR = 0; };
static GPIO_TypeDef gpio;
static GPIO_TypeDef *GPIOE = &gpio;
static Registers rcc, adc;
static Registers *RCC = &rcc, *ADC1 = &adc;
static const uint32_t RCC_AHB2ENR_GPIOEEN = 1;
static const int ON = 1, OFF = 2;
static uint32_t mask;
static uint32_t __get_PRIMASK() { return mask; }
static void __disable_irq() { mask = 1; }
static void __set_PRIMASK(uint32_t value) { mask = value; }
static void initPin_PP(GPIO_TypeDef *, int) {}
static void setPin_PP(GPIO_TypeDef *, int, int) {}
#include "ir_modem.c"

static std::vector<bool> stream() {
    ir_tx = {};
    ir_modem_init();
    ir_on();
    ir_off();
    std::vector<bool> levels(2048, false);
    for(int byte = 0; byte < 256; ++byte) {
        ir_tx_send_byte(byte);
        for(unsigned t = 0; t < 11 * 256; ++t)
            levels.push_back(ir_tx_advance() == ON);
    }
    levels.insert(levels.end(), 1024, false);
    return levels;
}

int main() {
    static_assert(sizeof(CyberBearReceiver) <= 80, "Receiver RAM budget exceeded");
    auto levels = stream();
    // Independent clocks, arbitrary sampling phase, polarity and tick wraparound.
    for(double step : {1.0, 48000.0 / (50000000.0 / 1041), 1.005, 0.995}) {
        for(unsigned phase = 0; phase < 16; ++phase) {
            CyberBearReceiver receiver(phase % 2 ? 0xfffff000u : 0);
            std::vector<uint8_t> bytes;
            for(double position = phase / 16.0; position < levels.size(); position += step) {
                uint8_t byte;
                bool level = levels[static_cast<unsigned>(position)] ^ (phase % 2);
                if(receiver.Sample(level, byte)) bytes.push_back(byte);
            }
            assert(bytes.size() == 256);
            for(int byte = 0; byte < 256; ++byte) assert(bytes[byte] == byte);
            auto stats = receiver.GetDiagnostics();
            assert(stats.edges > 0 && stats.fast_intervals > 0 && stats.slow_intervals > 0);
            assert(stats.packets == 256 && stats.headers >= 256);
        }
    }
    // No spurious packets from a fixed level, a continuous carrier or old envelopes.
    for(unsigned mode = 0; mode < 8; ++mode) {
        CyberBearReceiver receiver;
        uint32_t seed = 1;
        for(unsigned t = 0; t < 48000; ++t) {
            seed = seed * 1664525u + 1013904223u;
            bool level = mode == 1 || (mode == 2 && t % 4 < 2) ||
                         (mode == 3 && t % 32 < 16) ||
                         (mode == 4 && t % 144 < 30) ||
                         (mode == 5 && (seed >> 24) < 16) ||
                         (mode == 6 && t % 1024 < 128 && t % 4 < 2) ||
                         (mode == 7 && t % 4096 < 256 && t % 4 < 2);
            uint8_t byte;
            assert(!receiver.Sample(level, byte));
        }
    }
    // A truncated frame is rejected, and the next valid frame is received.
    CyberBearReceiver receiver;
    for(unsigned i = 0; i < 1500; ++i) {
        uint8_t byte;
        receiver.Sample(levels[2048 + i], byte);
    }
    for(unsigned i = 0; i < 1024; ++i) {
        uint8_t byte;
        assert(!receiver.Sample(false, byte));
    }
    std::vector<uint8_t> recovered;
    for(auto level : levels) {
        uint8_t byte;
        if(receiver.Sample(level, byte)) recovered.push_back(byte);
    }
    assert(recovered.size() == 256);
    for(unsigned i = 0; i < 256; ++i) assert(recovered[i] == i);
    assert(receiver.GetDiagnostics().rejected > 0);

    // Muted local TX is observable in diagnostics but cannot deliver self-echo.
    CyberBearReceiver muted;
    for(auto level : levels) {
        uint8_t byte;
        assert(!muted.Sample(level, byte, true));
    }
    assert(muted.GetDiagnostics().edges > 0 && muted.GetDiagnostics().packets == 0);
    std::cout << "A12 -> IRPCB: 256 bytes, clock mismatch, polarity, wrap, recovery, mute\n";
}
