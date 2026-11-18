#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
#include "ir.cpp"

static std::vector<std::pair<uint8_t, uint16_t>> packets;
static void received(uint8_t bits, uint16_t word) { packets.emplace_back(bits, word); }
static void pulse(uint32_t duration) {
    IRRcvr::tmr_rx.flags = 2;
    IRRcvr::tmr_rx.capture = duration;
    IRPCB_TEST_RX_IRQHandler();
}
static void timeout() {
    IRRcvr::tmr_rx.flags = 1;
    IRPCB_TEST_RX_IRQHandler();
}
static void sample(bool level) {
    // IR_PCB-F_Cu.gtl: DD1 pad 11, T1C1/PA1 is connected to /IR_RCVR.
    // PA3 is a different net, /UART_RX. Do not follow the driver's pin macro.
    if(level) Gpio::input_levels |= 1u << 1;
    else Gpio::input_levels &= ~(1u << 1);
    IRRcvr::tmr_rx.flags = 1;
    IRPCB_TEST_RX_IRQHandler();
}
static void frame(uint8_t byte, bool invert = false) {
    for(unsigned t = 0; t < 3072; ++t) {
        unsigned symbol = t / 256;
        unsigned period = symbol >= 2 && symbol < 10 &&
                          (byte & (0x80 >> (symbol - 2))) ? 32 : 4;
        bool level = symbol != 1 && symbol < 10 && t % period < period / 2;
        sample(level ^ invert);
    }
}
int main() {
    IRLed::Init();
    IRLed::SetCarrierFreq(56000);
    IRRcvr::callbackI = received;
    IRRcvr::Init();
    assert(settings.rx_mode == STANDARD_MODE);
    assert(IRRcvr::tmr_rx.capture_irq && !IRRcvr::tmr_rx.update_irq);
    pulse(2400);
    for(int bit = 7; bit >= 0; --bit) pulse(0xa5 & (1 << bit) ? 1200 : 600);
    timeout();
    assert(packets.size() == 1 && packets.back().first == 8 && packets.back().second == 0xa500);

    IRRcvr::SetMode(CYBERBEAR_MODE);
    assert(settings.rx_mode == CYBERBEAR_MODE && settings.tx_mode == STANDARD_MODE);
    assert(IRRcvr::tmr_rx.frequency == 48000);
    assert(!IRRcvr::tmr_rx.capture_irq && IRRcvr::tmr_rx.update_irq);
    for(unsigned t = 0; t < 1024; ++t) sample(false);
    frame(0x96);
    assert(packets.size() == 2 && packets.back().first == 8 && packets.back().second == 0x96);
    auto d = IRRcvr::GetDiagnostics();
    assert(d.samples == 4096 && d.packets == 1 && d.fast_intervals && d.slow_intervals);
    pulse(2400); // Capture flags in sampler mode cannot advance or decode anything.
    assert(IRRcvr::GetDiagnostics().samples == d.samples);
    for(unsigned t = 0; t < 1024; ++t) {
        Gpio::input_levels ^= 1u << 3; // UART traffic must not appear as IR edges.
        sample(false);
    }
    assert(IRRcvr::GetDiagnostics().edges == d.edges);

    // Local TX mutes incoming bytes, while diagnostics still observe the pin.
    IRLed::TransmitWord(0xffff, 16, 90, nullptr);
    frame(0x55);
    assert(packets.size() == 2);
    Sys::tick_ms = 100;
    IRLed::dma_tx.callback(nullptr, 2);
    assert(IRLed::IsEchoSuppressed(138, 39));
    assert(!IRLed::IsEchoSuppressed(139, 39));
    frame(0x55);
    assert(packets.size() == 3 && packets.back().second == 0x55);

    // Pending framing and diagnostics are cleared when switching back.
    IRRcvr::SetMode(STANDARD_MODE);
    assert(IRRcvr::GetDiagnostics().samples == 0);
    pulse(2400);
    for(unsigned bit = 0; bit < 16; ++bit) pulse(600);
    assert(packets.size() == 4 && packets.back().first == 16 && packets.back().second == 0);
    IRRcvr::SetMode(42);
    assert(settings.rx_mode == STANDARD_MODE);
    primask = 1;
    IRRcvr::SetMode(CYBERBEAR_MODE);
    IRRcvr::GetDiagnostics();
    assert(primask == 1);
    primask = 0;

    // Echo timestamps use unsigned arithmetic across the millisecond wrap.
    Sys::Time_ = 0xfffffff0u;
    Sys::tick_ms = 0;
    IRLed::TransmitWord(0xffff, 16, 90, nullptr);
    IRLed::dma_tx.callback(nullptr, 2);
    assert(IRLed::IsEchoSuppressed(8, 39));
    assert(!IRLed::IsEchoSuppressed(24, 39));
    std::cout << "IRPCB RX driver: modes, legacy packets, raw bytes, diagnostics, echo\n";
}
