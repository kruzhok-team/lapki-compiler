#include "commonEars.hpp"

extern "C" {
    //
    // Потоковый декодер для передаваемых звуком данных
    // TODO: донастроить в условиях слабого сигнала
    //

    int BIT_INTERVAL = 595;
    int BIT_WINDOW = 250;

    // Полосовой фильтр, IIR, ~2.9 КГц
    static float filter_2_9_next(float x) {
        static float y_past[3] = {0.0f};
        static float x_past[3] = {0.0f};

        const float feedforw[] = {1.0/40.0, 1.0/80.0, -1.0/27.0};
        const float feedback[] = {1.0, -9.0/5.0, 11.0/12.0};

        x_past[2] = x_past[1];
        x_past[1] = x_past[0];
        x_past[0] = x;

        y_past[2] = y_past[1];
        y_past[1] = y_past[0];
        y_past[0] = feedforw[0] * x_past[0] +
                    feedforw[1] * x_past[1] +
                    feedforw[2] * x_past[2] -
                    feedback[1] * y_past[1] -
                    feedback[2] * y_past[2];

        return y_past[0];
    }

    // Полосовой фильтр, IIR, 6.0 КГц
    static float filter_6_0_next(float x) {
        static float y_past[3] = {0.0f};
        static float x_past[3] = {0.0f};

        const float feedforw[] = {1.0/33.0, 1.0/412.0, -1.0/31.0};
        const float feedback[] = {1.0, -11.0/8.0, 14.0/15.0};

        x_past[2] = x_past[1];
        x_past[1] = x_past[0];
        x_past[0] = x;

        y_past[2] = y_past[1];
        y_past[1] = y_past[0];
        y_past[0] = feedforw[0] * x_past[0] +
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
        int is_enabled;
        // TODO: чувствительность как параметр
    } acoustic_rx = { .fn = preamble_detect, 0 };

    static void preamble_detect(float u, float v) {
        if (ABS(v) > 200.0f) {
            acoustic_rx.preamble_ttl = 100;
            acoustic_rx.preamble_loc = 100;
            acoustic_rx.preamble_max = ABS(v);
            acoustic_rx.fn = preamble_sync;
        }
    }

    static void preamble_sync(float u, float v) {
        if (acoustic_rx.preamble_ttl >= 0) {
            acoustic_rx.preamble_ttl--;

            if (ABS(v) > acoustic_rx.preamble_max) {
                acoustic_rx.preamble_max = ABS(v);
                acoustic_rx.preamble_loc = 100 - acoustic_rx.preamble_ttl;
            }
            return;
        }

        if (acoustic_rx.preamble_loc >= 0) {
            acoustic_rx.preamble_loc--;
            return;
        }

        acoustic_rx.training_ttl = BIT_WINDOW;
        acoustic_rx.training_max = 0.0f;
        acoustic_rx.fn = link_training;
    }

    static void link_training(float u, float v) {
        if (acoustic_rx.training_ttl >= 0) {
            acoustic_rx.training_ttl--;

            if (ABS(u) > acoustic_rx.training_max) {
                acoustic_rx.training_max = ABS(u);
            }
            return;
        }

        acoustic_rx.bit_thresh = 0.75f * acoustic_rx.training_max;
        acoustic_rx.bit_pend = 8;
        acoustic_rx.bit_ttl = BIT_WINDOW;
        acoustic_rx.bit_holdoff = BIT_INTERVAL - BIT_WINDOW;
        acoustic_rx.bit_max = 0.0f;
        acoustic_rx.bit_acc = 0;
        acoustic_rx.fn = bit_sampling;
    }

    static void bit_sampling(float u, float v) {
        if (acoustic_rx.bit_holdoff >= 0) {
            acoustic_rx.bit_holdoff--;
            return;
        }

        if (acoustic_rx.bit_ttl >= 0) {
            acoustic_rx.bit_ttl--;

            if (ABS(u) > acoustic_rx.bit_max) {
                acoustic_rx.bit_max = ABS(u);
            }
            return;
        }

        int bit_detected = (acoustic_rx.bit_max >= acoustic_rx.bit_thresh);

        acoustic_rx.bit_pend--;
        acoustic_rx.bit_acc += (bit_detected << acoustic_rx.bit_pend);

        if (acoustic_rx.bit_pend > 0) {
            acoustic_rx.bit_holdoff = BIT_INTERVAL - BIT_WINDOW;
            acoustic_rx.bit_ttl = BIT_WINDOW;
            acoustic_rx.bit_max = 0.0f;
            return;
        }

        acoustic_rx.fn = preamble_detect;
        acoustic_rx.have_byte = 1;
    }

    int acoustic_rx_is_available() {
        return acoustic_rx.have_byte;
    }

    int acoustic_rx_read_byte() {
        acoustic_rx.have_byte = 0;
        return acoustic_rx.bit_acc & 0b11111111;
    }

    // Диспетчеризирующая функция
    // u     2.9 кГц сигнал
    // v     6.0 кГц сигнал
    static void rx_advance(float u, float v) {
        if (acoustic_rx.fn) {
            acoustic_rx.fn(u, v);
        }
        acoustic_rx.tick++;
    }
}
