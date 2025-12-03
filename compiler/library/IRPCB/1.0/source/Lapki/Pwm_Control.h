// PwmSetup s = { GPIOB, 1, TIM3, 4, invInverted, omPushPull, 255 };
//
// pin.set скважность/duty cycle
// pin.setFrequency частота
#ifndef PWMOUTPUT_H
#define PWMOUTPUT_H

#include "app_classes.h"
#include "gd_lib.h"

#define PIN_LED1 PA6
#define PIN_LED2 PA7
#define PIN_LED3 PB0
#define PIN_LED4 PB1
#define PIN_FRONT_LED1 PB8
#define PIN_FRONT_LED2 PB9

//Нельзя использовать пины одного таймера для ввода/вывод, только один режим у всех пинов таймера

// PA6 PA7 PB8 PB9 PB0 PB1
class PwmOutput {
    const PinOutputPWM_t pin;

   public:
    // example — PwmOutputPin(Gpio3);
    // default top_value — 255
    // you need manually initialise frequency, otherwise it is ub, or the
    // frequency of last pin with same Timer
    PwmOutput(GPIO_TypeDef* pgpio, uint16_t apin, Gpio::OutMode mode);

    // TODO надо бы это переименовать/сделать статическим методом, так как
    // меняется частота не пина, а таймера

    // changes Frequency of all pins with same Timer (connected to hardware)
    void setFrequency(uint32_t hz) { pin.SetFrequencyHz(hz); }

    // 255 — 100%
    // 0 — 0%
    void setDuty(uint32_t val);

    // 0 - 1
    //  step is 0.004 for top_value — 255
    void setDutyPortion(float portion);
};
#if 0
void PwmInputTimCallbackAdv(void *p);

class PwmInputPinAdvanced : private TimHw {
private:
    GPIO_TypeDef *pgpio;
    uint16_t pin_n;
    Gpio::PullUpDown pull_up_down;
    VirtualTimer vtmr;
    
    friend void PwmInputTimCallbackAdv(void *p);

    void TimerCallback();

    void setupChannelPair(uint8_t base_ch, ChnlMode rising_mode, ChnlMode falling_mode);
    
public:
    volatile int32_t pwm_duty=0;
    volatile int32_t pwm_frequency_hz = 0;
    PwmInputPinAdvanced(GPIO_TypeDef *apgpio, uint16_t apin, Gpio::PullUpDown apull_up_down,
            TIM_TypeDef *aptimer) : TimHw(aptimer),
                pgpio(apgpio), pin_n(apin), pull_up_down(apull_up_down) {}

    void Init();
};


// PA6 PA7 PB8 PB9 PB0 PB1
class PwmInput {
    PwmInputPinAdvanced pin;

   public:
    volatile int32_t& pwm_duty = pin.pwm_duty;
    volatile int32_t& pwm_frequency = pin.pwm_frequency_hz;

    PwmInput(GPIO_TypeDef* pgpio, uint16_t apin, Gpio::PullUpDown mode);
};
#endif


#endif