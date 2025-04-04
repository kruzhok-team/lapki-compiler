#pragma once

#include "CommonSound.hpp"
#include "Notes.hpp"

namespace detail {

    namespace speaker {

        // defined in SpeakerSound
        // bool isInit = false;
    }
}

class SpeakerNote {

public:

    SpeakerNote() {

        if (!detail::speaker::isInit) {

            mrx::hal::speaker::init();

            mrx::hal::pwm::enablePWMTIM2();

            detail::speaker::isInit = true;
        }
    }

    // TODO: duration in ms - DEFAULT PARAM
    // TODO:В ПЛАТФОРМЕ ЕГО НЕ ДОЛЖНО БЫТЬ
    void play(Note *note, const uint16_t duration) {

        // TODO: Если не работает, то вернуть тип на тип Sound* и не парить мозги
        mrx::hal::speaker::startSound(reinterpret_cast<Sound*>(&note), duration);
    }

    bool isSoundEnd() {

        return mrx::hal::speaker::soundController.sound == nullptr;
    }
};