/*
 * ChunkTypes.h
 *
 *  Created on: 08 Jan 2015
 *      Author: Kreyl
 */

#ifndef CHUNK_TYPES_H_
#define CHUNK_TYPES_H_

#include "color.h"
#include "MsgQ.h"

enum class Chunk {Setup, Wait, Goto, End, Repeat};

// ==== Different types of chunks ====
struct BaseChunk { //  Everyone must contain this.
    Chunk chunk_sort;
    union {
        uint32_t value;
        uint32_t volume;
        uint32_t time_ms;
        uint32_t chunk_to_jump_to;
        int32_t repeat_cnt;
    };
};

// RGB LED chunk
struct LedRGBChunk : public BaseChunk {
    Color_t color;
};

// HSV LED chunk
struct LedHSVChunk : public BaseChunk {
    ColorHSV_t color;
};

// LED Smooth
struct LedSmoothChunk : public BaseChunk {
    uint32_t brightness;
};

// Beeper
struct BeepChunk : public BaseChunk {
    uint32_t freq_Hz;
    uint32_t freq_smooth = 0;
};


#if 1 // ====================== Base sequencer class ===========================
void VTmrUniversalCb(void* p);

template <class TChunk>
class BaseSequencer : private IrqHandler {
protected:
    enum SequencerLoopTask {sltProceed, sltBreak};
    VirtualTimer itmr;
    const TChunk *pstart_chunk, *pcurrent_chunk, *inext_chunk = nullptr;
    int32_t repeat_counter = -1;
    EvtMsg ievt_msg;
    virtual void ISwitchOff() = 0;
    virtual SequencerLoopTask ISetup() = 0;
    void SetupDelay(uint32_t ms) { itmr.SetI(TIME_MS2I(ms), VTmrUniversalCb, this); }

    // Process sequence
    void IIrqHandlerI() {
        if(itmr.IsArmedX()) itmr.ResetI();  // Reset timer
        while(true) {   // Process the sequence
            switch(pcurrent_chunk->chunk_sort) {
                case Chunk::Setup: // setup now and exit if required
                    if(ISetup() == sltBreak) return;
                    break;

                case Chunk::Wait: { // Start timer, pointing to next chunk
                        uint32_t delay = pcurrent_chunk->time_ms;
                        pcurrent_chunk++;
                        if(delay != 0) {
                            SetupDelay(delay);
                            return;
                        }
                    }
                    break;

                case Chunk::Repeat:
                    if(repeat_counter == -1) repeat_counter = pcurrent_chunk->repeat_cnt;
                    if(repeat_counter == 0) {    // All was repeated, goto next
                        repeat_counter = -1;     // reset counter
                        pcurrent_chunk++;
                    }
                    else {  // repeating in progress
                        pcurrent_chunk = pstart_chunk;  // Always from beginning
                        repeat_counter--;
                    }
                    break;

                case Chunk::Goto:
                    pcurrent_chunk = pstart_chunk + pcurrent_chunk->chunk_to_jump_to;
                    if(ievt_msg.id != EvtId::None) evt_q_main.SendNowOrExitI(ievt_msg);
                    SetupDelay(1);
                    return;
                    break;

                case Chunk::End:
                    if(ievt_msg.id != EvtId::None) evt_q_main.SendNowOrExitI(ievt_msg);
                    if(inext_chunk == nullptr) { // There is nothing next
                        pstart_chunk = nullptr;
                        pcurrent_chunk = nullptr;
                        return;
                    }
                    else { // There is something next
                        repeat_counter = -1;
                        pstart_chunk = inext_chunk;
                        pcurrent_chunk = inext_chunk;
                        inext_chunk = nullptr;
                    }
                    break;
            } // switch
        } // while
    } // IProcessSequenceI
public:
    void SetupSeqEndEvt(EvtMsg AEvtMsg) { ievt_msg = AEvtMsg; }

    void StartOrRestartI(const TChunk *PChunk) {
        repeat_counter = -1;
        pstart_chunk = PChunk;   // Save first chunk
        pcurrent_chunk = PChunk;
        inext_chunk = nullptr;
        IIrqHandlerI();
    }

    void StartOrRestart(const TChunk *PChunk) {
        Sys::Lock();
        StartOrRestartI(PChunk);
        Sys::Unlock();
    }

    void StartOrContinue(const TChunk *PChunk) {
        if(PChunk == pstart_chunk) return; // Same sequence
        else StartOrRestart(PChunk);
    }

    void StopI() {
        if(pstart_chunk != nullptr) {
            if(itmr.IsArmedX()) itmr.ResetI();
            pstart_chunk = nullptr;
            pcurrent_chunk = nullptr;
            inext_chunk = nullptr;
        }
        ISwitchOff();
    }

    void Stop() {
        Sys::Lock();
        StopI();
        Sys::Unlock();
    }

    const TChunk* GetCurrentSequence() { return pstart_chunk; }

    // Next sequence will be started after current ends
    void SetNextSequenceI(const TChunk *PChunk) {
        if(IsIdle() and PChunk != nullptr) StartOrRestartI(PChunk);
        else inext_chunk = PChunk;
    }

    bool IsIdle() { return (pstart_chunk == nullptr and pcurrent_chunk == nullptr); }
};
#endif

#endif // CHUNK_TYPES_H_
