/*
 * beeper.h
 *
 *  Created on: 22 ����� 2015 �.
 *      Author: Kreyl
 */

#ifndef BEEPER_H__
#define BEEPER_H__

#include "BaseSequencer.h"
#include "gd_lib.h"

class Beeper : public BaseSequencer<BeepChunk> {
private:
    const PinOutputPWM_t ipin;
    uint32_t curr_freq = 0;
    void ISwitchOff() { ipin.Set(0); }
    SequencerLoopTask ISetup() {
        ipin.Set(pcurrent_chunk->volume);
        if(pcurrent_chunk->freq_smooth == 0) { // If smooth time is zero, set now
            curr_freq = pcurrent_chunk->freq_Hz;
            ipin.SetFrequencyHz(curr_freq);
            pcurrent_chunk++;   // goto next
        }
        else {
            if     (curr_freq < pcurrent_chunk->freq_Hz) curr_freq += 100;
            else if(curr_freq > pcurrent_chunk->freq_Hz) curr_freq -= 100;
            ipin.SetFrequencyHz(curr_freq);
            // Check if completed now
            if(curr_freq == pcurrent_chunk->freq_Hz) pcurrent_chunk++;
            else { // Not completed
                // Calculate time to next adjustment
                uint32_t Delay = (pcurrent_chunk->freq_smooth / (curr_freq+4)) + 1;
                SetupDelay(Delay);
                return sltBreak;
            } // Not completed
        }
        return sltProceed;
    }
public:
    Beeper(const PwmSetup APinSetup) : BaseSequencer(), ipin(APinSetup) {}
    void Init() { ipin.Init(); }
    void Beep(uint32_t Freq_Hz, uint8_t Volume) {
        ipin.SetFrequencyHz(Freq_Hz);
        ipin.Set(Volume);
    }
    void Off() { ipin.Set(0); }
};

#endif //BEEPER_H__
