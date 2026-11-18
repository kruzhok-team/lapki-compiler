#pragma once

// Streaming port of bear_opt.zip: signal_reciever.ipynb / demo.
// TIM17 supplies 48 kHz: 0 = 12 kHz, 1 = 1.5 kHz, 256 samples/symbol.
// Frame: one 0-frequency header, one silent symbol, eight data bits (MSB first).
// Keep the acoustic receiver and the public IR / NoseBytes interfaces separate.
extern "C" {
    // Настройка чувствительности ИК-приёмника. После изменения пересобрать прошивку.
    // Пороги относятся к энергии после фильтров, а не к сырым значениям АЦП.
    static constexpr float IR_RX_START_THRESHOLD = 40.0f; // Минимум для старта кадра.
    static constexpr float IR_RX_BIT_THRESHOLD = 40.0f;   // Минимум для чтения бита.
    // Отношение краёв к пику старта: меньше — строже, больше — мягче (от 0 до 1).
    static constexpr float IR_RX_START_PEAK_RATIO = 0.5f;

    enum {
        IR_BIT_SAMPLES = 256,
        IR_DECIMATION = 16,
        IR_BUFFER_SIZE = 64,
        IR_DETECT_STEP = 15,
        IR_STARTUP_MARGIN = 70,
        IR_QUEUE_SIZE = 16,
        IR_TX_CYBERBEAR = 0,
        IR_TX_IRPCB = 1
    };

    // Optional backend, registered only by the IRPCB LED component.
    static void (*ir_pcb_start)(uint16_t, unsigned) = nullptr;
    static void (*ir_pcb_stop)() = nullptr;

    static struct {
        int initialized;
        int turn_mode;
        unsigned frame_sample;
        volatile unsigned gap;
        volatile int active;
        unsigned protocol;
        uint8_t byte;
        // Packed descriptor: payload[15:0], bit count[20:16], protocol[21].
        volatile uint32_t queue[IR_QUEUE_SIZE];
        volatile unsigned head;
        volatile unsigned tail;
    } ir_tx = {};

    static struct {
        int is_enabled;
        unsigned phase;
        unsigned decimation;
        unsigned margin;
        float previous_input;
        float previous_highpass;
        float lowpass[4];
        float lowpass_twice[4];
        float envelope[2];
        float history[2][IR_BUFFER_SIZE];
        unsigned end;
        int reading;
        unsigned read_count;
        unsigned bit_count;
        uint8_t byte;
        volatile uint8_t queue[IR_QUEUE_SIZE];
        volatile unsigned head;
        volatile unsigned tail;
    } ir_rx = {};

    // These operations run in the foreground; restore the caller's IRQ state.
    static void ir_set_mode(int on) {
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        if (ir_pcb_stop)
            ir_pcb_stop();
        ir_tx.turn_mode = on;
        ir_tx.active = 0;
        ir_tx.gap = IR_BIT_SAMPLES;
        ir_tx.tail = ir_tx.head;
        setPin_PP(GPIOE, 12, on ? ON : OFF);
        __set_PRIMASK(mask);
    }

    static void ir_off() { ir_set_mode(0); }
    static void ir_on() { ir_set_mode(1); }

    static void ir_tx_enqueue(uint16_t word, unsigned bits, unsigned protocol) {
        uint32_t mask = __get_PRIMASK();
        __disable_irq();
        ir_tx.turn_mode = 0;
        unsigned next = (ir_tx.head + 1) % IR_QUEUE_SIZE;
        // A full queue drops the new byte, never corrupts the frame in flight.
        if (next != ir_tx.tail) {
            ir_tx.queue[ir_tx.head] = word | (bits << 16) | (protocol << 21);
            ir_tx.head = next;
        }
        __set_PRIMASK(mask);
    }

    static void ir_tx_send_byte(int byte) {
        ir_tx_enqueue(static_cast<uint8_t>(byte), 8, IR_TX_CYBERBEAR);
    }

    // Return the LED level for exactly one sample; no waits in the IRQ.
    static int ir_tx_advance() {
        if (ir_tx.turn_mode)
            return ON;
        if (!ir_tx.active) {
            if (ir_tx.gap) {
                --ir_tx.gap;
                return OFF;
            }
            if (ir_tx.tail == ir_tx.head)
                return OFF;
            uint32_t packet = ir_tx.queue[ir_tx.tail];
            ir_tx.tail = (ir_tx.tail + 1) % IR_QUEUE_SIZE;
            ir_tx.protocol = (packet >> 21) & 1;
            ir_tx.byte = static_cast<uint8_t>(packet);
            ir_tx.frame_sample = 0;
            ir_tx.active = 1;
            if (ir_tx.protocol == IR_TX_IRPCB) {
                if (ir_pcb_start)
                    ir_pcb_start(static_cast<uint16_t>(packet), (packet >> 16) & 31);
                else
                    ir_tx.active = 0;
                return OFF;
            }
        }
        if (ir_tx.protocol == IR_TX_IRPCB)
            return OFF; // TIM16 owns PE12 until this packet is complete.

        unsigned symbol = ir_tx.frame_sample / IR_BIT_SAMPLES;
        unsigned period = 4;
        if (symbol >= 2 && (ir_tx.byte & (0x80 >> (symbol - 2))))
            period = 32;
        int level = symbol != 1 && ir_tx.frame_sample % period < period / 2;
        if (++ir_tx.frame_sample == 10 * IR_BIT_SAMPLES) {
            ir_tx.active = 0;
            ir_tx.gap = IR_BIT_SAMPLES;
        }
        return level ? ON : OFF;
    }

    static float ir_history(unsigned channel, unsigned index) {
        return ir_rx.history[channel][(ir_rx.end + index) % IR_BUFFER_SIZE];
    }

    static void ir_rx_decimate(float energy_0, float energy_1) {
        ir_rx.envelope[0] = 0.5f * (ir_rx.envelope[0] + energy_0);
        ir_rx.envelope[1] = 0.5f * (ir_rx.envelope[1] + energy_1);
        for (unsigned channel = 0; channel < 2; ++channel)
            ir_rx.history[channel][ir_rx.end] = ir_rx.envelope[channel];
        ir_rx.end = (ir_rx.end + 1) % IR_BUFFER_SIZE;

        const unsigned center = IR_BUFFER_SIZE / 2;
        float zero = ir_history(0, center);
        float one = ir_history(1, center);
        if (ir_rx.margin < IR_STARTUP_MARGIN) {
            ++ir_rx.margin;
            return;
        }

        if (!ir_rx.reading && zero > IR_RX_START_THRESHOLD &&
            zero * IR_RX_START_PEAK_RATIO > ir_history(0, center - IR_DETECT_STEP) &&
            zero * IR_RX_START_PEAK_RATIO > ir_history(0, center + IR_DETECT_STEP) &&
            zero >= ir_history(0, center - 1) &&
            zero >= ir_history(0, center + 1)) {
            ir_rx.reading = 1;
            ir_rx.read_count = 0;
            ir_rx.bit_count = 0;
            ir_rx.byte = 0;
        }

        if (!ir_rx.reading)
            return;
        // Notebook positions: 32, 48, ..., 144 decimated samples after header.
        if (ir_rx.read_count == (2 + ir_rx.bit_count) *
                                (IR_BIT_SAMPLES / IR_DECIMATION)) {
            // A missing carrier is an incomplete frame, not a logical one.
            if (zero <= IR_RX_BIT_THRESHOLD && one <= IR_RX_BIT_THRESHOLD) {
                ir_rx.reading = 0;
                return;
            }
            ir_rx.byte = (ir_rx.byte << 1) | (zero <= one ? 1 : 0);
            if (++ir_rx.bit_count == 8) {
                unsigned next = (ir_rx.head + 1) % IR_QUEUE_SIZE;
                if (next != ir_rx.tail) {
                    ir_rx.queue[ir_rx.head] = ir_rx.byte;
                    ir_rx.head = next;
                }
                ir_rx.reading = 0;
            }
        }
        ++ir_rx.read_count;
    }

    static void ir_rx_advance(float sample) {
        // HPF, tau = 64: a0=(1+exp(-1/64))/2, a1=-a0, b1=exp(-1/64).
        float signal = 0.9922482185f * (sample - ir_rx.previous_input) +
                       0.9844964370f * ir_rx.previous_highpass;
        ir_rx.previous_input = sample;
        ir_rx.previous_highpass = signal;
        ir_rx.phase = (ir_rx.phase + 1) % 32;
        unsigned phase_0 = ir_rx.phase % 4;
        float products[4] = {
            phase_0 >= 2 ? signal : -signal,
            (phase_0 + 1) % 4 < 2 ? signal : -signal,
            ir_rx.phase >= 16 ? signal : -signal,
            (ir_rx.phase + 8) % 32 < 16 ? signal : -signal
        };
        for (unsigned i = 0; i < 4; ++i) {
            ir_rx.lowpass[i] += (products[i] - ir_rx.lowpass[i]) / 48.0f;
            ir_rx.lowpass_twice[i] +=
                (ir_rx.lowpass[i] - ir_rx.lowpass_twice[i]) / 48.0f;
        }
        // The notebook decimates the first processed sample, then every 16th.
        if (ir_rx.decimation == 0) {
            const float *v = ir_rx.lowpass_twice;
            ir_rx_decimate(v[0] * v[0] + v[1] * v[1],
                           v[2] * v[2] + v[3] * v[3]);
        }
        ir_rx.decimation = (ir_rx.decimation + 1) % IR_DECIMATION;
    }

    int ir_rx_is_available() {
        return ir_rx.head != ir_rx.tail;
    }

    int ir_rx_read_byte() {
        if (ir_rx.head == ir_rx.tail)
            return 0;
        int byte = ir_rx.queue[ir_rx.tail];
        ir_rx.tail = (ir_rx.tail + 1) % IR_QUEUE_SIZE;
        return byte;
    }

    void ir_modem_init() {
        if (ir_tx.initialized)
            return;
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
        initPin_PP(GPIOE, 12);
        setPin_PP(GPIOE, 12, OFF);
        ir_tx.initialized = 1;
    }

    void ir_modem() {
        // NoseBytes enables reception only after the photodiode ADC is ready.
        if (ir_rx.is_enabled)
            ir_rx_advance(ADC1->DR);
        if (ir_tx.initialized) {
            int level = ir_tx_advance();
            if (!(ir_tx.active && ir_tx.protocol == IR_TX_IRPCB))
                setPin_PP(GPIOE, 12, level);
        }
    }
}
