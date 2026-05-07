#include "LapkiLed.h"
#include <stdexcept>
#include <unordered_map>
#include "yartos.h"
#include "../lib/led.h"
#include "../lib/BaseSequencer.h"
#include "Sequences.h"
#include "board.h"
#include "Serial.h"

uint32_t absDif(uint32_t a, uint32_t b) { return (a >= b ? a - b : b - a); }

LedMultiSmooth::SequencerLoopTask LedMultiSmooth::ISetup() {
    if (mode == Fade) {
        if (chunksBrDifs.find(pcurrent_chunk) == chunksBrDifs.end())
            chunksBrDifs[pcurrent_chunk] =
                absDif(pcurrent_chunk->brightness, curr_value);
        // brightness = pcurrent_chunk->brightness;
        // brightness_diff = chunksBrDifs[pcurrent_chunk];
        // cv = curr_value;
        if (curr_value != pcurrent_chunk->brightness) {
            if (pcurrent_chunk->value == 0) {  // If smooth time is zero,
                curr_value = pcurrent_chunk->brightness;
                SetCurrent();
                chunksBrDifs.erase(pcurrent_chunk);
                pcurrent_chunk++;  // and goto next chunk
            } else {
                uint32_t step =
                    (chunksBrDifs[pcurrent_chunk] + pcurrent_chunk->value - 1) /
                    pcurrent_chunk->value;
                if (curr_value < pcurrent_chunk->brightness)
                    curr_value += step;
                else if (curr_value > pcurrent_chunk->brightness)
                    curr_value -= step;
                if (absDif(curr_value, pcurrent_chunk->brightness) < step)
                    curr_value = pcurrent_chunk->brightness;
                SetCurrent();
                // Check if completed now
                if (curr_value == pcurrent_chunk->brightness) {
                    chunksBrDifs.erase(pcurrent_chunk);
                    pcurrent_chunk++;
                } else {  // Not completed
                    // Calculate time to next adjustment
                    uint32_t delay = (pcurrent_chunk->value +
                                      chunksBrDifs[pcurrent_chunk] - 1) /
                                     chunksBrDifs[pcurrent_chunk];
                    // TODO delay > SYS_ST_TIMEDELTA: redo with adaptable step
                    // and time, SYS_ST_TIMEDELTA is the smallest period of
                    // waiting
                    if (delay > 0) SetupDelay(delay);
                    return sltBreak;
                }  // Not completed
            }  // if time > 256
        }  // if color is different
        else {
            chunksBrDifs.erase(pcurrent_chunk);
            pcurrent_chunk++;  // Color is the same, goto next chunk
        }
    } else {  // TODO redo with virtual timers
        if (curr_value == 0) {
            curr_value = ledOn;
            SetCurrent();
            SetupDelay(blinkt);
            return sltBreak;
        } else {
            curr_value = ledOff;
            SetCurrent();
            SetupDelay(blinkT - blinkt);
            return sltBreak;
        }
    }
    return sltProceed;
}

// LED
Led::Led(uint8_t num) {
    switch (num) {  // можно было сделать, через массив
        case 1:
            ls_ = new LedMultiSmooth(LED_PWM1);
            break;
        case 2:
            ls_ = new LedMultiSmooth(LED_PWM2);
            break;
        case 3:
            ls_ = new LedMultiSmooth(LED_PWM3);
            break;
        case 4:
            ls_ = new LedMultiSmooth(LED_PWM4);
            break;
        case 5:
            ls_ = new LedMultiSmooth(LED_FRONT1);
            break;
        case 6:
            ls_ = new LedMultiSmooth(LED_FRONT2);
            break;
        default:
            Serial::print("RE: Wrong led index\n");
            exit(1);
    }
    ls_->Init();
}

Led::Led(PwmSetup apin_setup, uint32_t afreq) {
    ls_ = new LedMultiSmooth(apin_setup, afreq);
    ls_->Init();
}

void Led::set(uint32_t val) {
    if(!isReady()) return;
    ls_->Set(val);
    isActive = val != 0;
    state = val;
}

void Led::fadeTo(uint32_t ms, uint32_t brightness) {
    if(!isReady()) return;
    ls_->mode = Fade;
    pattern->value = ms;
    pattern->brightness = brightness;
    ls_->StartOrRestart(pattern);
    isActive = brightness != 0;
    state = brightness;
    lastStartMs = Sys::GetSysTime();
    intervalMs = ms;
}

bool Led::isReady() {
    return Sys::TimeElapsedSince(lastStartMs) >= intervalMs;
}

void Led::blink(uint32_t t, uint32_t T) {
    if(!isReady()) return;
    ls_->blinkt = t;
    ls_->blinkT = T;
    ls_->mode = Blinker;
    ls_->StartOrRestart(pattern);
}

// LedMultiBlinker
LedMultiBlinker::SequencerLoopTask LedMultiBlinker::ISetup() {
    if (!isActive) {
        isActive = !isActive;
        On();
        SetupDelay(blinkt);
        return sltBreak;
    } else {
        isActive = !isActive;
        Off();
        SetupDelay(blinkT - blinkt);
        return sltBreak;
    }
    return sltProceed;
}

// SystemLed
SystemLed::SystemLed() {
    lb_ = new LedMultiBlinker(LUMOS_PIN);
    lb_->Init();
}

void SystemLed::blink(uint32_t t, uint32_t T) {
    stop();
    lb_->blinkt = t;
    lb_->blinkT = T;
    lb_->StartOrRestart(lbsqStart);
}