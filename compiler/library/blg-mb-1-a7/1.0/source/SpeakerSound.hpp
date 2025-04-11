#pragma once

#include "Sounds.hpp"

namespace detail {

    namespace speaker {

        bool isInit = false;
    }
}

class SpeakerSound {

public:

    SpeakerSound() {

        if (!detail::speaker::isInit) {

            mrx::hal::speaker::init();

            mrx::hal::pwm::enablePWMTIM2();

            detail::speaker::isInit = true;
        }
    }

    // duration in ms
    void play(Sound *sound, const uint16_t duration) {

        mrx::hal::speaker::startSound(sound, duration);
    }

    void stop() {

        mrx::hal::speaker::stopSound();
    }

    bool isSoundEnd() {

        return mrx::hal::speaker::soundController.sound == nullptr;
    }
};