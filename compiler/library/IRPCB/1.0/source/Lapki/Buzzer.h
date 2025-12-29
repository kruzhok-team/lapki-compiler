#ifndef BUZZER_H
#define BUZZER_H

#include "Sequences.h"
#include "beeper.h"

// from InitFunctions.hpp
extern Beeper beeper;

// probably you need beeper, this is just a wrapper
namespace Buzzer {

// TODO Можно было бы добавить изменение диапазона, но оставшийся код написан на
// макросах и его не получиться поменять в рантайме
constexpr uint8_t top_val = BEEPER_TOP;

// 0 <= Volume <= BEEPER_TOP = 22
void beepSpecific(uint32_t Freq_Hz, uint8_t Volume) {
    if (Volume > top_val) Volume = top_val;
    beeper.Beep(Freq_Hz, Volume);
}

void beep() { beeper.StartOrRestart(bsqBeep); }

// излишне: функция beep занимает довольно мало времени
// bool isAvailable() { return beeper.IsIdle(); }

};  // namespace Buzzer
#endif