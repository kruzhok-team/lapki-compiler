#pragma once

#include "Sounds.hpp"

namespace detail {

    namespace speaker {
        bool isPlayed = false;
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
    void play(Sound *sound, const uint32_t duration) {
        detail::speaker::isPlayed = true;
        mrx::hal::speaker::startSound(sound, duration);

        // mrx::hal::speaker::startSound(&LaughterSound, duration);
    }

    void stop() {

        mrx::hal::speaker::stopSound();
    }

    bool isSoundEnd() {
        const bool isEnd = detail::speaker::isPlayed && mrx::hal::speaker::soundController.sound == nullptr;
        detail::speaker::isPlayed = false;
        return isEnd;
    }
};