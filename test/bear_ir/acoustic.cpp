#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include "acoustic_modem.c"

static void reset() {
    acoustic_rx = {};
    acoustic_rx.fn = preamble_detect;
    acoustic_rx.is_enabled = 1;
    // Let the production IIR filters settle between independent frames.
    stm32g431::ears::adcArray[0] = 2048;
    for (unsigned i = 0; i < 4096; ++i) acoustic_modem();
}

static std::vector<int> transmit(unsigned byte, float amplitude, unsigned phase,
                                 bool reset_receiver = true) {
    if (reset_receiver) reset();
    std::vector<int> received;
    uint32_t noise = 1;
    for (unsigned t = 0; t < 6500; ++t) {
        // Existing acoustic format: 6 kHz start, 2.9 kHz training,
        // then eight OOK symbols, MSB first, at nominal 48 kHz sampling.
        float frequency = 0;
        if (t < 200) frequency = 6000;
        else if (t < 600) frequency = 2900;
        else if (t < 600 + 8 * 595 && (byte & (128 >> ((t - 600) / 595))))
            frequency = 2900;
        noise = noise * 1664525u + 1013904223u;
        float sample = 2048 + (int(noise >> 24) - 128) * 0.02f;
        if (frequency)
            sample += amplitude * std::sin(6.28318530718f * frequency * (t + phase) / 48000);
        stm32g431::ears::adcArray[0] = static_cast<uint16_t>(sample);
        acoustic_modem();
        if (acoustic_rx_is_available()) received.push_back(acoustic_rx_read_byte());
    }
    return received;
}

int main() {
    reset();
    rx_advance(0, 300); // A weak 6 kHz candidate must not capture the decoder.
    assert(acoustic_rx.fn == preamble_detect && !acoustic_rx_is_available());

    reset();
    rx_advance(0, 800); // A loud spike followed by quiet training is not a packet.
    for (unsigned t = 0; t < 7000; ++t) {
        rx_advance(t % 2 ? 20 : -20, 0);
        assert(!acoustic_rx_is_available());
    }
    assert(acoustic_rx.fn == preamble_detect);
    assert(transmit(0x96, 900, 3, false) == std::vector<int>{0x96});

    // Weak complete audio frames, silence and bounded ambient noise stay quiet.
    assert(transmit(0xff, 250, 0).empty());
    for (unsigned t = 0; t < 48000; ++t) {
        stm32g431::ears::adcArray[0] = 2048 + (t % 2 ? 20 : -20);
        acoustic_modem();
        assert(!acoustic_rx_is_available());
    }
    for (unsigned byte = 0; byte < 256; ++byte)
        assert(transmit(byte, 900, byte % 8) == std::vector<int>{int(byte)});
    std::cout << "Acoustic RX: weak starts, missing training, noise, 256 strong audio frames\n";
}
