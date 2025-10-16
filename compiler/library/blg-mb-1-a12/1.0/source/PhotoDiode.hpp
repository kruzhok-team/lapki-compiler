#pragma once
namespace detail {

    namespace PtohotoDiode {

        bool isInit = false;
    }
}

#define DIODE_ON 1
#define DIODE_OFF 0

class PhotoDiode {

    bool isEvent { false };

    bool isEventSetting { false };
    
    public:
    uint16_t threshold{};
    uint8_t mode = DIODE_OFF;
    uint16_t value;

    PhotoDiode() {

        if (!detail::PtohotoDiode::isInit) {

            mrx::hal::photoDiode::init();

            mrx::hal::photoDiode::start();

            detail::PtohotoDiode::isInit = true;
        }
        on();
    }

    // call this function in loop()
    // not user func
    void scan() {

        value = mrx::hal::photoDiode::getSense();

        if (mode == DIODE_ON && isEventSetting) {

            if (value > threshold) {
                isEvent = true;
            }
        }
    }

    void on() {
        mode = DIODE_ON;
    }

    void off() {
        mode = DIODE_OFF;
    }

    bool isThresholdValue() {

        auto copy = isEvent;
        isEvent = false;

        return copy;
    }

    void setupEvent(const uint16_t value) {

        threshold = value;
        isEventSetting = true;
        isEvent = false;
    }
};