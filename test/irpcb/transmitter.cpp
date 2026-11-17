#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include "ir.cpp"

// Run the production A12 decoder against samples produced by the IRPCB driver.
struct GPIO_TypeDef {};
struct AdcRegisters { uint32_t DR = 0; };
struct RccRegisters { uint32_t AHB2ENR = 0; };
static GPIO_TypeDef gpio;
static GPIO_TypeDef *GPIOE = &gpio;
static AdcRegisters adc;
static AdcRegisters *ADC1 = &adc;
static RccRegisters rcc;
static RccRegisters *RCC = &rcc;
static const int ON = 1, OFF = 2;
static const uint32_t RCC_AHB2ENR_GPIOEEN = 1;
static void initPin_PP(GPIO_TypeDef *, uint8_t) {}
static void setPin_PP(GPIO_TypeDef *, uint8_t, int) {}
#include "ir_modem.c"

static int callbacks;
static void completed() { ++callbacks; }
static std::vector<uint8_t> output() {
    auto p = static_cast<const uint8_t *>(IRLed::dma_tx.memory);
    return {p, p + IRLed::dma_tx.count};
}
static void finish() {
    if (IRLed::tx_busy) assert(IRLed::dma_tx.completion_irq);
    IRLed::dma_tx.callback(nullptr, 2);
    assert(!IRLed::dma_tx.enabled && !IRLed::sampling_tmr.enabled);
}
static std::vector<uint8_t> standard(uint16_t data, int bits, unsigned freq, uint8_t power) {
    std::vector<uint8_t> result;
    auto pulses = [&](unsigned duration, bool carrier) {
        unsigned pairs = (duration * freq + 1999999) / 2000000;
        for (unsigned i = 0; i < pairs * 2; ++i) {
            result.push_back(carrier ? power : 0);
            result.push_back(0);
        }
    };
    pulses(2400, true);
    pulses(600, false);
    for (int i = 0; i < bits; ++i) {
        pulses(data & 0x8000 ? 1200 : 600, true);
        pulses(600, false);
        data <<= 1;
    }
    pulses(2400, false);
    return result;
}

int main() {
    IRLed::Init();
    IRLed::SetCarrierFreq(56000);
    static_assert(sizeof(IRLed::dac_buf) == 3996, "Shared buffer grew");
    settings.tx_mode = STANDARD_MODE;
    for (unsigned freq : {30000, 56000}) {
        IRLed::SetCarrierFreq(freq);
        for (int bits = 1; bits <= 16; ++bits) {
            for (uint16_t word : {0, 0xffff, 0x9696}) {
                IRLed::TransmitWord(word, bits, 90, completed);
                assert(IRLed::sampling_tmr.frequency == freq * 2);
                assert(output() == standard(word, bits, freq, 90));
                assert(IRLed::dma_tx.memory == IRLed::dac_buf);
                finish();
            }
        }
    }

    ir_modem_init();
    ir_on();
    ir_off();
    ir_tx_send_byte(0);
    for (unsigned i = 0; i < 2048; ++i) ir_rx_advance(2048);
    uint32_t seed = 1;
    settings.tx_mode = CYBERBEAR_MODE;
    for (int byte = 0; byte < 256; ++byte) {
        int before = callbacks;
        IRLed::TransmitWord(0x5a00 | byte, 16, 90, completed);
        assert(callbacks == before);
        assert(IRLed::sampling_tmr.frequency == 48000);
        assert(IRLed::dma_tx.memory == IRLed::dac_buf);
        auto samples = output();
        assert(samples.size() == 3072);
        for (unsigned t = 0; t < samples.size(); ++t) {
            unsigned sample = t;
            unsigned symbol = sample / 256;
            unsigned period = symbol >= 2 && symbol < 10 &&
                              (byte & (0x80 >> (symbol - 2))) ? 32 : 4;
            uint8_t expected = symbol != 1 && symbol < 10 && sample % period < period / 2 ? 90 : 0;
            assert(samples[t] == expected);
        }
        const double ratio = (50000000.0 / 1041) / 48000;
        for (double pos = 0; pos < samples.size(); pos += ratio) {
            seed = seed * 1664525u + 1013904223u;
            float noise = (static_cast<int>(seed >> 24) - 128) * 0.1f;
            ir_rx_advance(2048.0f + samples[static_cast<unsigned>(pos)] * 3.0f + noise);
        }
        // Drain the A12 history while preserving the next frame's start time.
        finish();
        assert(callbacks == before + 1);
        for (unsigned t = 0; t < 1024; ++t) ir_rx_advance(2048);
        assert(ir_rx_is_available());
        assert(ir_rx_read_byte() == byte);
        assert(!ir_rx_is_available());
    }

    // Back-to-back frames at the real timer rate: 50 MHz / floor(50 MHz / 48 kHz).
    // Resample the actual IRPCB waveform at the A12 receiver's 48 kHz rate.
    std::vector<uint8_t> stream;
    for (int byte = 0; byte < 256; ++byte) {
        IRLed::TransmitWord(byte, 16, 90, completed);
        auto samples = output();
        stream.insert(stream.end(), samples.begin(), samples.end());
        finish();
    }
    ir_rx = {};
    for (unsigned i = 0; i < 2048; ++i) ir_rx_advance(2048);
    const double ratio = (50000000.0 / (50000000 / 48000)) / 48000;
    std::vector<int> received;
    for (double pos = 0; pos < stream.size(); pos += ratio) {
        ir_rx_advance(2048.0f + stream[static_cast<unsigned>(pos)] * 3.0f);
        if (ir_rx_is_available()) received.push_back(ir_rx_read_byte());
    }
    for (unsigned i = 0; i < 1024; ++i) {
        ir_rx_advance(2048);
        if (ir_rx_is_available()) received.push_back(ir_rx_read_byte());
    }
    assert(received.size() == 256);
    for (int byte = 0; byte < 256; ++byte) assert(received[byte] == byte);

    // Overlapping calls, including a mode switch, cannot overwrite active DMA.
    settings.tx_mode = CYBERBEAR_MODE;
    IRLed::TransmitWord(0x96, 8, 255, completed);
    auto active = output();
    IRLed::dma_tx.callback(nullptr, 0); // Stale IRQ with no completion flag.
    assert(IRLed::tx_busy && IRLed::dma_tx.enabled);
    int before = callbacks;
    settings.tx_mode = STANDARD_MODE;
    IRLed::SetCarrierFreq(30000);
    IRLed::TransmitWord(0xffff, 16, 1, nullptr);
    assert(output() == active && IRLed::sampling_tmr.frequency == 48000);
    assert(callbacks == before);
    finish();
    assert(callbacks == before + 1);
    IRLed::TransmitWord(0xffff, 16, 1, completed);
    assert(IRLed::sampling_tmr.frequency == 60000);
    assert(output() == standard(0xffff, 16, 30000, 1));
    finish();

    // Abort releases ownership and does not deliver a stale completion callback.
    settings.tx_mode = CYBERBEAR_MODE;
    IRLed::TransmitWord(0xa5, 8, 50, completed);
    before = callbacks;
    IRLed::ResetI();
    assert(!IRLed::tx_busy && !IRLed::dma_tx.enabled);
    finish();
    assert(callbacks == before);
    IRLed::TransmitWord(0x55, 8, 1, completed);
    for (auto value : output()) assert(value == 0 || value == 1);
    finish();

    // Unsupported modes, word lengths and carrier frequencies fail safely.
    settings.tx_mode = 42;
    IRLed::TransmitWord(0x55, 8, 90, completed);
    assert(!IRLed::tx_busy && !IRLed::dma_tx.enabled);
    settings.tx_mode = STANDARD_MODE;
    for (int bits : {-1, 0, 17}) {
        IRLed::TransmitWord(0xffff, bits, 90, completed);
        assert(!IRLed::tx_busy && !IRLed::dma_tx.enabled);
    }
    IRLed::SetCarrierFreq(0);
    IRLed::SetCarrierFreq(0xffffffffu);
    IRLed::TransmitWord(0x9696, 16, 90, completed);
    assert(IRLed::sampling_tmr.frequency == 60000);
    finish();
    std::cout << "IRPCB: old waveform preserved; 256 bytes decoded by A12; shared buffer 3996 bytes\n";
}
