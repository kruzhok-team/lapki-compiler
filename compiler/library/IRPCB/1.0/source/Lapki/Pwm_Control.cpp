#include "Pwm_Control.h"

#include <vector>

#include "Serial.h"
#include "core_cm4.h"
#include "gd32e11x_kl.h"
#include "gd_lib.h"
constexpr uint32_t top_value = 255;
using timChannel = std::pair<TIM_TypeDef*, uint8_t>;

// XXX PwmInput не работает,
/*
INTF при каналах 0 и 1 всегда 25
код:
PwmInput in(PA6, Gpio::PullUp);
эксперимент: замкнуть контакты светодиода скрепкой
*/

enum usageMode { none = 0, input = 1, output = 2 };

// сделано по примеру TimHw::SetChnlMode и PinOutputPWM_t::Init
usageMode getChannelMode(TIM_TypeDef* tim, uint8_t ch) {
    // здесь только проверка включения, не конфигурации
    if (tim->CHCTL2 & (1UL << (ch * 4))) return usageMode::output;

    switch (ch) {
        case 0:
            if ((tim->CHCTL0 & 0x0003) != 0) return usageMode::input;
        case 1:
            if ((tim->CHCTL0 & 0x0300) != 0) return usageMode::input;
        case 2:
            if ((tim->CHCTL1 & 0x0003) != 0) return usageMode::input;
        case 3:
            if ((tim->CHCTL1 & 0x0300) != 0) return usageMode::input;
        default:
            return usageMode::none;
    }
}

timChannel getTimChannel(GPIO_TypeDef* pgpio, uint16_t apin) {
    if (pgpio == GPIOA) {
        if (apin == 6) return {TIM2, 0};
        if (apin == 7) return {TIM2, 1};
        if (apin == 8) return {TIM0, 1};
    } else if (pgpio == GPIOB) {
        if (apin == 8) return {TIM3, 2};
        if (apin == 9) return {TIM3, 3};
        if (apin == 0) return {TIM2, 2};
        if (apin == 1) return {TIM2, 3};
    }
    NVIC_SystemReset();
    return {TIM2, 0};
}

usageMode getTimerMode(TIM_TypeDef* tim) {
    // У таймера, по задумке, только один настроенный режим, но некоторые пины,
    // могут быть не настроены
    for (int i = 0; i < 4; i++)
        if (getChannelMode(tim, i) != usageMode::none)
            return getChannelMode(tim, i);
    return usageMode::none;
}

// system reset if pins are not PA6 PA7 PB8 PB9 PB0 PB10 or used or timer is
// different mode
PwmSetup getPwmSetup(GPIO_TypeDef* pgpio, uint16_t apin, Gpio::OutMode mode) {
    // getTimChannel resets system if pins are not supported
    timChannel timch = getTimChannel(pgpio, apin);
    if (getTimerMode(timch.first) == usageMode::input) NVIC_SystemReset();

    // if (getChannelMode(timch.first, timch.second) != usageMode::none) {
    //     NVIC_SystemReset();
    // }
    return PwmSetup(pgpio, apin, timch.first, timch.second, invNotInverted,
                    mode, top_value);
}

PwmOutput::PwmOutput(GPIO_TypeDef* pgpio, uint16_t apin, Gpio::OutMode mode)
    : pin(getPwmSetup(pgpio, apin, mode)) {
    pin.Init();
    setDuty(0);
}

void PwmOutput::setDuty(uint32_t val) {
    if (val > top_value) val = top_value;
    if (val < 0) val = 0;
    pin.Set(val);
}

void PwmOutput::setDutyPortion(float portion) {
    setDuty(static_cast<uint32_t>(portion * top_value));
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#if 0
void PwmInputPinAdvanced::setupChannelPair(uint8_t base_ch,
                                           ChnlMode rising_mode,
                                           ChnlMode falling_mode) {
    if (base_ch == 1 && TimHw::itmr == TIM0) {
        TimHw::SetInputActiveEdge(0, RiseFall::Rising);
        TimHw::SetChnlMode(0, rising_mode);
        TimHw::EnChnl(0);

        TimHw::SetInputActiveEdge(1, RiseFall::Falling);
        TimHw::SetChnlMode(1, falling_mode);
        TimHw::EnChnl(1);
    }
    // Стандартная конфигурация для каналов 0/1 и 2/3
    else {
        TimHw::SetInputActiveEdge(base_ch, RiseFall::Rising);
        TimHw::SetChnlMode(base_ch, rising_mode);
        TimHw::EnChnl(base_ch);

        TimHw::SetInputActiveEdge(base_ch + 1, RiseFall::Falling);
        TimHw::SetChnlMode(base_ch + 1, falling_mode);
        TimHw::EnChnl(base_ch + 1);
    }
}

void PwmInputTimCallbackAdv(void* p) {
    static_cast<PwmInputPinAdvanced*>(p)->TimerCallback();
}

// void PwmInputPinAdvanced::Init() {
//     Gpio::SetupInput(pgpio, pin_n, pull_up_down);
//     TimHw::Init();
//     TimHw::SetTopValue(0xFFFF);  // Maximum
//     ChnlMode rising_mode, falling_mode;
//     timChannel timch = getTimChannel(pgpio, pin_n);
//     if (timch.second == 0) {
//         rising_mode = ChnlMode::CI0FE0;
//         falling_mode = ChnlMode::CI0FE1;
//     } else if (timch.second == 2) {
//         rising_mode = ChnlMode::CI2FE2;
//         falling_mode = ChnlMode::CI2FE3;
//     } else if (timch.second == 1 && TimHw::itmr == TIM0) {
//         // gpio4
//         rising_mode = ChnlMode::CI0FE0;
//         falling_mode = ChnlMode::CI0FE1;
//     } else {
//         NVIC_SystemReset();
//     }
//     setupChannelPair(timch.second, rising_mode, falling_mode);
//     if (timch.second == 0 || (timch.second == 1 && TimHw::itmr == TIM0)) {
//         TimHw::SetTriggerInput(TimHw::TriggerIn::CI0FE0);
//     } else if (timch.second == 2) {
//         TimHw::SetTriggerInput(TimHw::TriggerIn::CI1FE1);
//     } else {
//         NVIC_SystemReset();
//         return;
//     }
//     TimHw::SelectSlaveMode(TimHw::SlaveMode::Restart);
//     TimHw::Enable();
//     vtmr.Set(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallbackAdv,
//              this);
// }
void PwmInputPinAdvanced::Init() {
    Gpio::SetupInput(pgpio, pin_n, pull_up_down);
    TimHw::Init();
    TimHw::SetTopValue(0xFFFF); // Maximum

    timChannel timch = getTimChannel(pgpio, pin_n);

    // В зависимости от таймера и базового канала, настраиваем соответствующую пару.
    if (timch.first == TIM0) {
        // Особый случай для TIM0, так как у него входы могут быть мультиплексированы по-разному.
        // Для PA8 (канал 1) мы используем его как вход TI1.
        // Пара для измерения: каналы 1 (период) и 0 (длительность).

        // Канал 1 (период) настраивается на захват по нарастающему фронту от входа TI1
        TimHw::SetInputActiveEdge(1, RiseFall::Rising);
        SET_BITS(TimHw::itmr->CHCTL0, 0b11UL, 0b01UL, 8); // CH1MOD = 01 (Input, IC1 is mapped on TI1)

        // Канал 0 (длительность) настраивается на захват по спадающему фронту от входа TI1
        TimHw::SetInputActiveEdge(0, RiseFall::Falling);
        SET_BITS(TimHw::itmr->CHCTL0, 0b11UL, 0b10UL, 0); // CH0MOD = 10 (Input, IC0 is mapped on TI1)

        // Включаем каналы 0 и 1
        TimHw::itmr->CHCTL2 |= (1UL << 4) | (1UL << 0); // CH1EN=1, CH0EN=1
        // Устанавливаем полярность для канала 0
        TimHw::itmr->CHCTL2 |= (1UL << 1); // CH0P = 1 (inverting, falling edge)

        // Триггер для сброса - нарастающий фронт на TI1
        TimHw::SetTriggerInput(TimHw::TriggerIn::CI1FE1);

    } else if (timch.first == TIM2) { // Случаи для TIM2 (PA6, PB0)
        switch (timch.second) {
            case 0: // Для PA6 (TIM2, CH0)
                SET_BITS(TimHw::itmr->CHCTL0, 0b11UL, 0b01UL, 0); // CH0MOD
                SET_BITS(TimHw::itmr->CHCTL0, 0b11UL, 0b10UL, 8); // CH1MOD
                TimHw::itmr->CHCTL2 = (1UL << 5) | (1UL << 4) | (1UL << 0);
                TimHw::SetTriggerInput(TimHw::TriggerIn::CI0FE0);
                break;

            case 2: // Для PB0 (TIM2, CH2)
                SET_BITS(TimHw::itmr->CHCTL1, 0b11UL, 0b01UL, 0); // CH2MOD
                SET_BITS(TimHw::itmr->CHCTL1, 0b11UL, 0b10UL, 8); // CH3MOD
                TimHw::itmr->CHCTL2 = (1UL << 13) | (1UL << 12) | (1UL << 8);
                TimHw::SetTriggerInput(TimHw::TriggerIn::ITI1);
                break;
        }
    } else {
        // Другие таймеры пока не поддерживаем
        NVIC_SystemReset();
    }

    // Общая часть для всех
    TimHw::SelectSlaveMode(TimHw::SlaveMode::Restart);
    TimHw::Enable();
    vtmr.Set(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallbackAdv, this);
}

// void PwmInputPinAdvanced::TimerCallback() {
//     Sys::LockFromIRQ();  // The callback is invoked outside the kernel critical
//                          // zone
//                          // Are there new values?
//         bool ch0_capture_occured = TimHw::itmr->INTF & TIM_INTF_CH0IF;
//         bool ch1_capture_occured = TimHw::itmr->INTF & TIM_INTF_CH1IF;
//     if (!ch0_capture_occured and !ch1_capture_occured) {  // No new values
//         if (pwm_duty != 0) {  // There was an impulse before and now it has gone
//             pwm_duty = 0;     // Means no pulses
//             pwm_frequency_hz = 0;
//             // evt_q_app.SendNowOrExitI(AppMsg(AppEvt::NewPwmData));
//         }
//     }
//     // Both width and period are captured. Ignore if only one value is done.
//     else if (ch0_capture_occured and ch1_capture_occured) {
//         // Calc new duty
//         int32_t period = TimHw::itmr->CH0CV;
//         int32_t pulse_width = TimHw::itmr->CH1CV;
//         int32_t new_pwm_duty =
//             (period == 0) ? 0 : (100UL * pulse_width) / period;
//         // Check if changed
//         int32_t diff = pwm_duty - new_pwm_duty;
//         if (diff < 0) diff = -diff;
//         if (diff > PWM_DUTY_DEVIATION_percent) {
//             pwm_duty = new_pwm_duty;
//             pwm_frequency_hz =
//                 (period == 0) ? 0
//                               : (Clk::GetTimInputFreq(TimHw::itmr) / period);
//             // evt_q_app.SendNowOrExitI(AppMsg(AppEvt::NewPwmData));
//         }
//     }
//     // Restart timer
//     vtmr.SetI(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallbackAdv,
//               this);
//     Sys::UnlockFromIRQ();
// }

void PwmInputPinAdvanced::TimerCallback() {
    Sys::LockFromIRQ();

    // Читаем флаги и сразу их сбрасываем
    uint32_t flags = TimHw::itmr->INTF;
    if (flags & (TIM_INTF_CH0IF | TIM_INTF_CH1IF | TIM_INTF_CH2IF | TIM_INTF_CH3IF)) {
        TimHw::itmr->INTF = 0;
    }

    bool period_captured = false;
    bool pulse_captured = false;
    int32_t period = 0;
    int32_t pulse_width = 0;

    timChannel timch = getTimChannel(pgpio, pin_n);

    if (timch.first == TIM0) {
        // Для TIM0 и PA8: период на CH1, длительность на CH0
        period_captured = flags & TIM_INTF_CH1IF;
        pulse_captured  = flags & TIM_INTF_CH0IF;
        if (period_captured && pulse_captured) {
            period = TimHw::itmr->CH1CV;
            pulse_width = TimHw::itmr->CH0CV;
        }
    } else { // Для TIM2
        if (timch.second == 0) { // Пара 0/1
            period_captured = flags & TIM_INTF_CH0IF;
            pulse_captured  = flags & TIM_INTF_CH1IF;
            if (period_captured && pulse_captured) {
                period = TimHw::itmr->CH0CV;
                pulse_width = TimHw::itmr->CH1CV;
            }
        } else if (timch.second == 2) { // Пара 2/3
            period_captured = flags & TIM_INTF_CH2IF;
            pulse_captured  = flags & TIM_INTF_CH3IF;
            if (period_captured && pulse_captured) {
                period = TimHw::itmr->CH2CV;
                pulse_width = TimHw::itmr->CH3CV;
            }
        }
    }

    if (period_captured && pulse_captured) {
        if (period > 10) {
            int32_t new_duty = (100UL * pulse_width) / period;
            if (new_duty <= 100) {
                pwm_duty = new_duty;
                pwm_frequency_hz = Clk::GetTimInputFreq(TimHw::itmr) / period;
            }
        }
    } else {
        // Проверка на таймаут (пропадание сигнала)
        if(TimHw::itmr->CNT > 40000) {
            pwm_duty = 0;
            pwm_frequency_hz = 0;
        }
    }

    vtmr.SetI(TIME_MS2I(INPUT_PWM_CHECK_PERIOD_ms), PwmInputTimCallbackAdv, this);
    Sys::UnlockFromIRQ();
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Только пины соответствующие нулевому/второму каналу таймеру, либо это GPIO4,
// таймер нужного режима
timChannel isInputPinAvailabe(GPIO_TypeDef* pgpio, uint16_t apin) {
    timChannel timch = getTimChannel(pgpio, apin);
    if (getTimerMode(timch.first) == usageMode::output) NVIC_SystemReset();

    if (timch.second == 0)
        if (!getChannelMode(timch.first, 0) && !getChannelMode(timch.first, 1))
            return timch;
    if (timch.second == 2)
        if (!getChannelMode(timch.first, 2) && !getChannelMode(timch.first, 3))
            return timch;
    // gpio4
    if (timch.first == TIM0 && timch.second == 1)
        if (!getChannelMode(timch.first, 0) && !getChannelMode(timch.first, 1))
            return timch;
    NVIC_SystemReset();
    return timch;
}

PwmInput::PwmInput(GPIO_TypeDef* pgpio, uint16_t apin, Gpio::PullUpDown mode)
    : pin(pgpio, apin, mode, isInputPinAvailabe(pgpio, apin).first) {
    pin.Init();
}
#endif