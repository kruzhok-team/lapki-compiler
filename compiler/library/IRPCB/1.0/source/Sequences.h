/*
 * Sequences.h
 *
 *  Created on: 2015
 *      Author: Kreyl
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include "BaseSequencer.h"

#if 1 // ======================== Simple LED blink =============================
#define BLINK_DELAY_MS      180
const BaseChunk lbsqStart[] = {
        {Chunk::Setup, 1},
        {Chunk::End}
};
#endif

#if 1 // =========================== LED Smooth ================================
#define LED_TOP_BRIGHTNESS  255

const LedSmoothChunk lsqFadeIn[] = {
        {Chunk::Setup, 207, LED_TOP_BRIGHTNESS},
        {Chunk::End}
};

const LedSmoothChunk lsqFadeInOut[] = {
        {Chunk::Setup, 207, LED_TOP_BRIGHTNESS},
        {Chunk::Setup, 207, 0},
        {Chunk::End}
};

const LedSmoothChunk lsqShot[] = {
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::Wait, 72},
        {Chunk::Setup, 0, 0},
        {Chunk::End}
};


#define RELOADING_BLINK_DELAY   72
const LedSmoothChunk lsqReloading[] = {
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, 0, 0},
        {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Goto, 0}
};

const LedSmoothChunk lsqMagazinesEnded[] = {
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::End}
};

const LedSmoothChunk lsqHit[] = {
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::Wait, 153},
        {Chunk::Setup, 0, 0},
        {Chunk::End}
};

const LedSmoothChunk lsqHitsEnded[] = {
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::End}
};

const LedSmoothChunk lsqUsbCmd[] = {
        {Chunk::Setup, 0, 0},
        {Chunk::Wait, 90},
        {Chunk::Setup, 0, LED_TOP_BRIGHTNESS},
        {Chunk::End}
};
#endif

#if 1 // ============================= Beeper ==================================
#define BEEP_VOLUME     1   // Maximum 10
#define BEEP_VOLUME_MAX 11

#if 1 // ==== Notes ====
#define La_2    880

#define Do_3    1047
#define Do_D_3  1109
#define Re_3    1175
#define Re_D_3  1245
#define Mi_3    1319
#define Fa_3    1397
#define Fa_D_3  1480
#define Sol_3   1568
#define Sol_D_3 1661
#define La_3    1720
#define Si_B_3  1865
#define Si_3    1976

#define Do_4    2093
#define Do_D_4  2217
#define Re_4    2349
#define Re_D_4  2489
#define Mi_4    2637
#define Fa_4    2794
#define Fa_D_4  2960
#define Sol_4   3136
#define Sol_D_4 3332
#define La_4    3440
#define Si_B_4  3729
#define Si_4    3951

// Length
#define OneSixteenth    90
#define OneEighth       (OneSixteenth * 2)
#define OneFourth       (OneSixteenth * 4)
#define OneHalfth       (OneSixteenth * 8)
#define OneWhole        (OneSixteenth * 16)

#define NOTE(Note, Dur) {Chunk::Setup, BEEP_VOLUME, Note}, {Chunk::Wait, Dur}, {Chunk::Setup, 0}, {Chunk::Wait, 18}
#endif

// MORSE
#define MORSE_TONE {Chunk::Setup, BEEP_VOLUME, Do_3}
#define MORSE_DOT_LENGTH 180
#define MORSE_DASH_LENGTH MORSE_DOT_LENGTH * 3
#define MORSE_PAUSE_LENGTH MORSE_DOT_LENGTH
#define MORSE_PAUSE {Chunk::Setup, 0}, {Chunk::Wait, MORSE_PAUSE_LENGTH}
#define MORSE_DOT MORSE_TONE, {Chunk::Wait, MORSE_DOT_LENGTH}, MORSE_PAUSE
#define MORSE_DASH MORSE_TONE, {Chunk::Wait, MORSE_DASH_LENGTH}, MORSE_PAUSE

// We are the champions
const BeepChunk bsqWeAreTheChampions[] = {
        NOTE(Fa_3, OneHalfth),
        NOTE(Mi_3, OneEighth),
        NOTE(Fa_3, OneEighth),
        NOTE(Mi_3, OneFourth),
        NOTE(Do_3, OneEighth),
        NOTE(Do_3, OneFourth),
        NOTE(La_2, OneEighth),
        NOTE(Re_3, OneHalfth),
        {Chunk::End}
};

// Type, BEEP_VOLUME, freq

const BeepChunk bsqShot[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 6000, 0},
        {Chunk::Wait, 40},
        {Chunk::Setup, BEEP_VOLUME_MAX, 1000, 18000},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqReloading[] = {
        {Chunk::Setup, BEEP_VOLUME, Si_3},   {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, 0},                   {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, BEEP_VOLUME, Re_D_4}, {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, 0},                   {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, BEEP_VOLUME, Fa_D_4}, {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Setup, 0},                   {Chunk::Wait, RELOADING_BLINK_DELAY},
        {Chunk::Goto, 0}
};

const BeepChunk bsqMagazReloaded[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 1950, 0},
        {Chunk::Wait, 40},
        {Chunk::Setup, BEEP_VOLUME_MAX, 2950, 120000},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqMagazEnded[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 2250, 0},
        {Chunk::Wait, 40},
        {Chunk::Setup, BEEP_VOLUME_MAX, 1050, 90000},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqHit[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 100, 0},
        {Chunk::Wait, 20},
        {Chunk::Setup, BEEP_VOLUME_MAX, 2700, 1000},
        {Chunk::Wait, 10},
        {Chunk::Setup, BEEP_VOLUME_MAX, 100, 1000},
        {Chunk::Wait, 100},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqIrPktRcvd[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 2700, 1000},
        {Chunk::Wait, 27},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqHitsEnded[] = {
        {Chunk::Setup, BEEP_VOLUME_MAX, 260, 0},
        {Chunk::Wait, 200},
        {Chunk::Setup, BEEP_VOLUME_MAX, 160, 0},
        {Chunk::Wait, 200},
        {Chunk::Setup, 0},
        {Chunk::Repeat, 3},
        {Chunk::End}
};

inline constexpr uint32_t kInterNotePause_ms = 27;
const BeepChunk bsqHitsAdded[] = { // XXX
        {Chunk::Setup, BEEP_VOLUME, Fa_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Mi_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Fa_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Mi_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Do_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Do_3, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, La_2, 0}, {Chunk::Wait, OneHalfth}, {Chunk::Setup, 0, 0, 0}, {Chunk::Wait, kInterNotePause_ms},
        {Chunk::Setup, BEEP_VOLUME, Re_3, 0}, {Chunk::Wait, OneHalfth},
        {Chunk::Setup, 0, 0, 0},
        {Chunk::End}
};

const BeepChunk bsqBeep[] = {
        {Chunk::Setup, 1, 1975},
        {Chunk::Wait, 54},
        {Chunk::Setup, 0},
        {Chunk::End}
};
const BeepChunk bsqBeepBeep[] = {
        {Chunk::Setup, BEEP_VOLUME, 1975},
        {Chunk::Wait, 54},
        {Chunk::Setup, 0},
        {Chunk::Wait, 54},
        {Chunk::Setup, BEEP_VOLUME, 1975},
        {Chunk::Wait, 54},
        {Chunk::Setup, 0},
        {Chunk::End}
};

#if 1 // ==== Extensions ====
// Pill
const BeepChunk bsqBeepPillOk[] = {
        {Chunk::Setup, BEEP_VOLUME, Si_3},
        {Chunk::Wait, 180},
        {Chunk::Setup, BEEP_VOLUME, Re_D_4},
        {Chunk::Wait, 180},
        {Chunk::Setup, BEEP_VOLUME, Fa_D_4},
        {Chunk::Wait, 180},
        {Chunk::Setup, 0},
        {Chunk::End}
};

const BeepChunk bsqBeepPillBad[] = {
        {Chunk::Setup, BEEP_VOLUME, Fa_4},
        {Chunk::Wait, 180},
        {Chunk::Setup, BEEP_VOLUME, Re_4},
        {Chunk::Wait, 180},
        {Chunk::Setup, BEEP_VOLUME, Si_3},
        {Chunk::Wait, 180},
        {Chunk::Setup, 0},
        {Chunk::End}
};
#endif // ext
#endif // beeper

#endif // SEQUENCES_H_
