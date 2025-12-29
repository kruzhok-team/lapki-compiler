/*
 * app_classes.h
 *
 *  Created on: 1.09.2024
 *      Author: Kreyl
 */

#ifndef APP_CLASSES_H_
#define APP_CLASSES_H_

#include "yartos.h"
#include "gd_lib.h"
#include "MsgQ.h"

#if 1 // ========================== Message queue ==============================
enum class AppEvt : uint8_t {
    Reset,
    StartFire, EndOfIrTx, EndOfDelayBetweenShots, MagazineReloadDone,
    IrRx,
    NewPwmData,
};

union AppMsg {
    uint32_t dw32;
    struct {
        uint16_t data16;
        uint8_t data8;
        AppEvt evt;
    } __attribute__((__packed__));

    AppMsg& operator = (const AppMsg &right) {
        dw32 = right.dw32;
        return *this;
    }
    AppMsg() : dw32(0) {}
    AppMsg(AppEvt aevt) : data16(0), data8(0), evt(aevt) {}
    AppMsg(AppEvt aevt, uint16_t adata16) : data16(adata16), evt(aevt) {}
    AppMsg(AppEvt aevt, uint8_t adata8, uint16_t adata16) : data16(adata16), data8(adata8), evt(aevt) {}
} __attribute__((__packed__));

using AppMsgQueue = EvtMsgQ<AppMsg, 9>;

extern AppMsgQueue evt_q_app;
#endif

#if 1 // ============================= Pulser pin ==============================
void PulserCallback(void *p);

class PulserPin: private Pin_t {
private:
    VirtualTimer itmr;
    friend void PulserCallback(void *p);
    void IOnTmrDone() { SetLo(); }
public:
    PulserPin(GPIO_TypeDef *apgpio, uint16_t apin) : Pin_t(apgpio, apin) {}
    void Init() { Pin_t::SetupOut(Gpio::PushPull); }
    void PulseI(uint32_t dur);
    void ResetI();
    void SetHi() { Pin_t::SetHi(); }
    void SetLo() { Pin_t::SetLo(); }
};
#endif

#if 1 // ============================ AppOutPin ================================
enum class PinMode {
    PushPullActiveHi=0, PushPullActiveLo=1,
    OpenDrainActiveHi=2, OpenDrainActiveLo=3
};

class CustomOutPin {
private:
    GPIO_TypeDef *pgpio;
    uint32_t pin_n;
    PinMode mode;
public:
    CustomOutPin(GPIO_TypeDef *apgpio, uint32_t apin) :
        pgpio(apgpio), pin_n(apin), mode(PinMode::PushPullActiveHi) {}

    void SetActive() {
        if(mode == PinMode::PushPullActiveHi or mode == PinMode::OpenDrainActiveHi)
            Gpio::SetHi(pgpio, pin_n);
        else
            Gpio::SetLo(pgpio, pin_n);
    }

    void SetInactive() {
        if(mode == PinMode::PushPullActiveHi or mode == PinMode::OpenDrainActiveHi)
            Gpio::SetLo(pgpio, pin_n);
        else
            Gpio::SetHi(pgpio, pin_n);
    }

    void SetActive(bool be_active) {
        if(be_active) SetActive();
        else SetInactive();
    }

    bool IsActive() {
        bool is_set_hi = pgpio->OCTL & (1UL << pin_n);
        if(mode == PinMode::PushPullActiveHi or mode == PinMode::OpenDrainActiveHi)
            return is_set_hi;
        else return !is_set_hi;
    }

    void SetMode(PinMode amode) {
        mode = amode;
        pgpio->SPD &= ~(1UL << pin_n); // No need in 120 MHz speed
        uint32_t ctl_mode = static_cast<uint32_t>(Gpio::Speed::speed10MHz) & 0b11UL;
        if(mode == PinMode::PushPullActiveHi or mode == PinMode::PushPullActiveLo)
            ctl_mode |= static_cast<uint32_t>(Gpio::OutMode::PushPull) << 2;
        else
            ctl_mode |= static_cast<uint32_t>(Gpio::OutMode::OpenDrain) << 2;
        pgpio->SetCtlMode(pin_n, ctl_mode);
    }

    void Init() { RCU->EnGpio(pgpio); }
};
#endif

#if 1 // ======================== Input pin: IRQ & Timed =======================
void InputPinIrqHandlerI();

class InputPinIrqTimed : public PinIrq {
private:
    systime_t start_time_st = 0;
public:
    InputPinIrqTimed(GPIO_TypeDef *apgpio, uint16_t apin, Gpio::PullUpDown apull_up_down):
        PinIrq(apgpio, apin, apull_up_down, InputPinIrqHandlerI) {}
    void Init() {
        PinIrq::Init(ttRising);
        CleanIrqFlag();
        EnableIrq(IRQ_PRIO_LOW);
    }
    bool CheckIfProcess() {
        if(Sys::TimeElapsedSince(start_time_st) < TIME_MS2I(INPUT_DEADTTIME_ms)) return false; // Not enough time passed
        else {
            start_time_st = Sys::GetSysTimeX();
            return true;
        }
    }
};
#endif

#if 1 // =========================== PWM input =================================
/* Here Input0 is used for restarting and capturing on the rising edge
and capturing on the falling one. Two outputs of EdgeDetector1 are used: CI0FE0 and CI0FE1.
          _________
   ______|         |________
         ^         ^
Input    CI0       CI1
EdgeOut: CI0FE0    CI0FE1
Capture: period    pulse width
         Trigger
         Restart
*/

void PwmInputTimCallback(void *p);

class PwmInputPin : private TimHw {
private:
    GPIO_TypeDef *pgpio;
    uint16_t pin_n;
    Gpio::PullUpDown pull_up_down;
    VirtualTimer vtmr;
    friend void PwmInputTimCallback(void *p);

    void TimerCallback() {
        Sys::LockFromIRQ(); // The callback is invoked outside the kernel critical zone
        // Are there new values?
        bool ch0_capture_occured = TimHw::itmr->INTF & TIM_INTF_CH0IF;
        bool ch1_capture_occured = TimHw::itmr->INTF & TIM_INTF_CH1IF;
        if(!ch0_capture_occured and !ch1_capture_occured) { // No new values
            if(pwm_duty != 0) { // There was an impulse before and now it has gone
                pwm_duty = 0; // Means no pulses
                evt_q_app.SendNowOrExitI(AppMsg(AppEvt::NewPwmData));
            }
        }
        // Both width and period are captured. Ignore if only one value is done.
        else if(ch0_capture_occured and ch1_capture_occured) {
            // Calc new duty
            int32_t period = TimHw::itmr->CH0CV;
            int32_t pulse_width = TimHw::itmr->CH1CV;
            int32_t new_pwm_duty = (period == 0)? 0 : (100UL * pulse_width) / period;
            // Check if changed
            int32_t diff = pwm_duty - new_pwm_duty;
            if(diff < 0) diff = -diff;
            if(diff > PWM_DUTY_DEVIATION_percent) {
                pwm_duty = new_pwm_duty;
                evt_q_app.SendNowOrExitI(AppMsg(AppEvt::NewPwmData));
            }
        }
        // Restart timer
        vtmr.SetI(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallback, this);
        Sys::UnlockFromIRQ();
    }
public:
    volatile int32_t pwm_duty=0;

    PwmInputPin(GPIO_TypeDef *apgpio, uint16_t apin, Gpio::PullUpDown apull_up_down,
            TIM_TypeDef *aptimer) : TimHw(aptimer),
                pgpio(apgpio), pin_n(apin), pull_up_down(apull_up_down) {}

    void Init() {
        Gpio::SetupInput(pgpio, pin_n, pull_up_down);
        TimHw::Init();
        TimHw::SetTopValue(0xFFFF); // Maximum
        // === Input0 === on the rising edge, perform a capture and restart
        TimHw::SetInputActiveEdge(0, RiseFall::Rising); // CI0FE0 is Active Rising (CI1FE0 also, but not used)
        // Setup input mode for Channel0: capture Input0 (IS0 = CI0FE0)
        TimHw::SetChnlMode(0, TimHw::ChnlMode::CI0FE0); // Chnl0 is input, capture on Input0's CI0FE0 signal
        // Restart timer on trigger; trigger is CI0FE0
        TimHw::SetTriggerInput(TimHw::TriggerIn::CI0FE0); // Use Input0's CI0FE0 as TriggerIn
        TimHw::SelectSlaveMode(TimHw::SlaveMode::Restart); // Configure slave mode controller in Restart mode
        TimHw::EnChnl(0); // Enable capture on channel 0
        // === Input1 === on the falling edge, perform capture
        TimHw::SetInputActiveEdge(1, RiseFall::Falling); // CI0FE1 is Active Falling (CI1FE1 also, but not used)
        // Setup input mode for Channel1: capture Input0 (IS1 = CI0FE1)
        TimHw::SetChnlMode(1, TimHw::ChnlMode::CI0FE1); // Chnl0 is input, capture on Input0's CI0FE1 signal
        TimHw::EnChnl(1); // Enable capture on channel 1
        // === Start Timer ===
        TimHw::Enable();
        vtmr.Set(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallback, this);
    }
};
#endif

#endif /* APP_CLASSES_H_ */
