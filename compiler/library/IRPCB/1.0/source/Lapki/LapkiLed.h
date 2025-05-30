#ifndef LED_H
#define LED_H
#include <unordered_map>

#include "../lib/BaseSequencer.h"
#include "../lib/led.h"
#include "Sequences.h"

constexpr int ledOn = 255;
constexpr int ledOff = 0;

enum LedMultiSmoothMode { Fade, Blinker };

// TODO redo with virtual timers


class LedMultiSmooth : public LedSmooth {
    using LedSmooth::LedSmooth;
    std::unordered_map<const LedSmoothChunk*, uint32_t> chunksBrDifs;

   public:
    // uint32_t brightness, cv, brightness_diff, delay, step;
    uint32_t blinkt, blinkT;
    LedMultiSmoothMode mode = Fade;
    virtual ~LedMultiSmooth() = default;

   protected:
    SequencerLoopTask ISetup() override;
};

class Led {
    LedSmoothChunk pattern[2] = {{Chunk::Setup, 0, 0}, {Chunk::End}};
    volatile bool isActive = 0;
    systime_t intervalMs = 0;
    systime_t lastStartMs = 0;
    int state = 0;
    LedMultiSmooth* ls_ = nullptr;

   public:
    LedMultiSmooth* const& ls = ls_;
    // номер светодиода
    // 1,2,3,4 — боковые, 5, 6 — передние
    Led(uint8_t num);

    Led(PwmSetup apin_setup, uint32_t afreq = 4294967295U);
    void set(uint32_t val);

    inline void on() { set(ledOn); }

    inline void off() { set(ledOff); }

    inline void toggle() {
        // isActive is changed inside
        if (isActive)
            off();
        else
            on();
    }

    void fadeTo(uint32_t ms, uint32_t brightness);

    inline void fadeIn(uint32_t ms) { fadeTo(ms, ledOn); }

    inline void fadeOut(uint32_t ms) { fadeTo(ms, ledOff); }

    inline void stop() {
        intervalMs = 0;
        ls_->Stop();
    }

    bool isReady();

    void blink(uint32_t t, uint32_t T);

    ~Led() { delete ls_; }
};

//////////////////////////////////////
/////////////////////////////////////

class LedMultiBlinker : public LedBlinker {
    using LedBlinker::LedBlinker;
    bool isActive = 0;
    inline static BeepChunk pattern[] = {{Chunk::Setup, 0, 0}, {Chunk::End}};

   public:
    uint32_t blinkt, blinkT;
    virtual ~LedMultiBlinker() {};

    inline void On() {
        LedBlinker::On();
        isActive = 1;
    }

    inline void Off() {
        LedBlinker::Off();
        isActive = 0;
    }

   protected:
    SequencerLoopTask ISetup() override;
};

class SystemLed {
    LedMultiBlinker* lb_;

   public:
    LedMultiBlinker* const& lb = lb_;

    SystemLed();

    inline void on() {
        stop();
        lb_->On();
    }

    inline void off() {
        stop();
        lb_->Off();
    }

    inline void stop() {
        lb_->Stop(); }

    void blink(uint32_t t, uint32_t T);

    ~SystemLed() {
        delete lb_;
        lb_ = NULL;
    }
};

#endif  // LED_HPP