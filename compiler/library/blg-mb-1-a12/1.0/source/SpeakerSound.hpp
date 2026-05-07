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
    Sound* currentSound = nullptr;
    uint32_t currentDuration = 0.;

    SpeakerSound() {
        if (!detail::speaker::isInit) {
            mrx::hal::speaker::init();
            mrx::hal::pwm::enablePWMTIM2();
            detail::speaker::isInit = true;
        }
    }
    // duration in ms
    void setupSound(Sound *sound, const uint32_t duration) {
        currentSound = sound;
        currentDuration = duration;
    }

    void play() {
        detail::speaker::isPlayed = true;
        mrx::hal::speaker::startSound(currentSound, currentDuration);
    }

    void stop() {
        mrx::hal::speaker::stopSound();
    }

    bool isSoundEnd() {
        const bool isEnd = detail::speaker::isPlayed && mrx::hal::speaker::soundController.sound == nullptr;
        if (isEnd) {
            detail::speaker::isPlayed = false;
        }
        return isEnd;
    }
};