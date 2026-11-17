#pragma once

#include <cstdint>

// Decoder for a DIGITAL input preserving the 12 kHz / 1.5 kHz carrier.
// A narrow-band remote-control receiver stripping that carrier is incompatible.
// Called at nominally 48 kHz; all storage is fixed, no allocation or ADC needed.
class CyberBearReceiver {
public:
    struct Diagnostics {
        uint32_t samples = 0;
        uint32_t edges = 0;
        uint32_t fast_intervals = 0;
        uint32_t slow_intervals = 0;
        uint32_t headers = 0;
        uint32_t packets = 0;
        uint32_t rejected = 0;
    };

    explicit CyberBearReceiver(uint32_t start_tick = 0) : tick(start_tick) {}

    Diagnostics GetDiagnostics() const { return diagnostics; }

    // Both polarities work: frequency is inferred from edge spacing.
    // Muting during local TX preserves diagnostics but discards framing state.
    bool Sample(bool level, uint8_t &result, bool muted = false) {
        ++diagnostics.samples;
        ++tick;
        if(!initialized) {
            initialized = true;
            previous_level = level;
            last_edge = tick;
        }
        bool edge = level != previous_level;
        uint32_t interval = tick - last_edge;
        bool fast = edge && interval >= 1 && interval <= 3;
        bool slow = edge && interval >= 12 && interval <= 20;
        if(edge) {
            ++diagnostics.edges;
            if(fast) ++diagnostics.fast_intervals;
            if(slow) ++diagnostics.slow_intervals;
        }

        if(muted) {
            reading = false;
            header_active = false;
        }
        else if(reading) {
            uint32_t elapsed = tick - origin;
            uint32_t window_start = (2 + bit_count) * kSymbol + kSymbol / 4;
            uint32_t window_end = window_start + kSymbol / 2;
            if(elapsed >= window_start && elapsed <= window_end) {
                if(fast) ++fast_edges;
                if(slow) ++slow_edges;
            }
            if(elapsed >= window_end) {
                bool zero = fast_edges >= kMinFastEdges;
                bool one = slow_edges >= kMinSlowEdges;
                if(zero == one) { // No carrier or ambiguous/mixed carriers.
                    ++diagnostics.rejected;
                    reading = false;
                }
                else {
                    byte = static_cast<uint8_t>((byte << 1) | (one ? 1 : 0));
                    ++bit_count;
                    fast_edges = slow_edges = 0;
                    if(bit_count == 8) {
                        reading = false;
                        ++diagnostics.packets;
                        result = byte;
                        if(edge) last_edge = tick;
                        previous_level = level;
                        return true;
                    }
                }
            }
        }
        else {
            if(fast) {
                if(!header_active) {
                    header_active = true;
                    header_first = last_edge;
                    header_edges = 1;
                }
                header_last = tick;
                // Saturation prevents a long noise train from wrapping a count.
                if(header_edges < 255) ++header_edges;
            }
            else if(edge) header_active = false;

            // Slow-carrier gaps are at most 20 ticks; require genuine silence.
            if(header_active && tick - header_last >= kHeaderQuiet) {
                uint32_t duration = header_last - header_first + 2;
                header_active = false;
                if(duration >= kHeaderMin && duration <= kHeaderMax &&
                        header_edges >= kMinHeaderEdges) {
                    ++diagnostics.headers;
                    origin = header_first;
                    reading = true;
                    bit_count = 0;
                    byte = 0;
                    fast_edges = slow_edges = 0;
                }
                else ++diagnostics.rejected;
            }
        }
        if(edge) last_edge = tick;
        previous_level = level;
        return false;
    }

private:
    static constexpr uint32_t kSymbol = 256;
    static constexpr uint32_t kHeaderMin = 224;
    static constexpr uint32_t kHeaderMax = 288;
    static constexpr uint32_t kHeaderQuiet = 48;
    static constexpr uint32_t kMinHeaderEdges = 80;
    static constexpr uint32_t kMinFastEdges = 24;
    static constexpr uint32_t kMinSlowEdges = 4;

    Diagnostics diagnostics;
    uint32_t tick = 0;
    uint32_t last_edge = 0;
    uint32_t header_first = 0;
    uint32_t header_last = 0;
    uint32_t origin = 0;
    uint16_t fast_edges = 0;
    uint16_t slow_edges = 0;
    uint8_t header_edges = 0;
    uint8_t bit_count = 0;
    uint8_t byte = 0;
    bool initialized = false;
    bool previous_level = false;
    bool header_active = false;
    bool reading = false;
};
