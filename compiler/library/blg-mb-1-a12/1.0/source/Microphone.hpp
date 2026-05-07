#pragma once

namespace detail {

    namespace microphone {

        bool isInit = false;
    }
}

#define MICROPHONE_ON 1
#define MICROPHONE_OFF 0

class Microphone {
    uint16_t lValue{}, rValue{};
    
    bool isEvent { false };
    
    uint8_t mode = MICROPHONE_OFF; 
    
    bool isEventSetting { false };
    uint16_t threshold{};
    unsigned long _previous{};
    unsigned long cooldown{};
public:

    // ctor
    Microphone(uint16_t _cooldown = 200) {
        cooldown = _cooldown;
        if (!detail::microphone::isInit) {

            mrx::hal::microphone::api::init();

            mrx::hal::microphone::detail::initDetector();

            detail::microphone::isInit = true;
        }
        on();
    }

    void scan () {

        senseLeft();
        senseRight();
    }

    bool isLoudSound() {
        if (mode == MICROPHONE_ON &&
            mrx::hal::microphone::detectedLevel > threshold && 
            millis() - _previous > cooldown
        ) {
            _previous = millis();
            isEvent = true;
        }

        auto copy = isEvent;
        isEvent = false;

        
        mrx::hal::microphone::detail::resetDetector();
        

        return copy;
    }

    void on() {
        mode = MICROPHONE_ON;
    }

    void off() {
        mode = MICROPHONE_OFF;
    }

    void setCooldown(unsigned long _cooldown) {
        cooldown = _cooldown;
    }

    void setupEvent(const uint16_t value) {

        threshold = value;
        isEvent = false;

        mrx::hal::microphone::detail::resetDetector();
        mrx::hal::microphone::detail::enableDetector(true);
        // isEventSetting = true;
    }

    // not user functions

    void senseLeft() {
        lValue = mrx::hal::microphone::api::senseLeft();
    }

    void senseRight() {
        rValue = mrx::hal::microphone::api::senseRight();
    }
};