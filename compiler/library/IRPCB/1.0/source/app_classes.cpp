/*
 * app_classes.cpp
 *
 *  Created on: 2.09.2024
 *      Author: Kreyl
 */

#include "app_classes.h"

#if 1 // ============================= Pulser pin ==============================
void PulserCallback(void *p) { static_cast<PulserPin*>(p)->IOnTmrDone(); }

void PulserPin::PulseI(uint32_t dur) {
    itmr.ResetI();
    SetHi();
    if(dur == 0) SetLo();
    else itmr.SetI(TIME_MS2I(dur), PulserCallback, (void*)this);
}
void PulserPin::ResetI() {
    itmr.ResetI();
    SetLo();
}
#endif

#if 1 // =========================== PWM input =================================
void PwmInputTimCallback(void *p) {
    static_cast<PwmInputPin*>(p)->TimerCallback();
}
#endif
