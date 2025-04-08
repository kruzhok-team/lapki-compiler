#pragma once

namespace detail {

    namespace microphone {

        bool isInit = false;
    }
}

class Microphone {

    uint16_t lValue{}, rValue{};

    bool isEvent { false };

    bool isEventSetting { false };
    uint16_t threshold{};

public:

    // ctor
    Microphone() {

        if (!detail::microphone::isInit) {

            mrx::hal::microphone::api::init();

            detail::microphone::isInit = true;
        }
    }

    void scan () {

        senseLeft();
        senseRight();

        if (isEventSetting) {

            if (lValue > threshold || rValue > threshold) {

                isEvent = true;
            }
        }
    }

    bool isLoudSound() {

        auto copy = isEvent;
        isEvent = false;

        return copy;
    }

    void setupEvent(const uint16_t value) {

        threshold = value;
        isEventSetting = true;
        isEvent = false;
    }

    // not user functions

    void senseLeft() {
        lValue = mrx::hal::microphone::api::senseLeft();
    }

    void senseRight() {
        rValue = mrx::hal::microphone::api::senseRight();
    }
};