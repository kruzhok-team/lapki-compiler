#pragma once

#include "SpeakerSound.hpp"
#include "CommonSound.hpp"

namespace detail {

    namespace speaker {
        // defined in SpeakerSound
        // bool isInit = false;
    }

    namespace math {

        // Аппроксимация Бхаскары (простая и быстрая, точность ~2%)
        // x — угол в градусах от 0 до 180
        float fast_sin_bhaskara(float x) {

            const float pi = 3.14159265358979323846f;
            // Переводим угол в диапазон 0..180
            while (x < 0) x += 360;
            while (x > 360) x -= 360;
            bool neg = false;
            if (x > 180) {
                x -= 180;
                neg = true;
            }
            float y = 4 * x * (180 - x) / (40500 - x * (180 - x));
            return neg ? -y : y;
        }
    }

    namespace note {

        const uint16_t y0 = 2047;
        const float pi = 3.1415926;

        void setUpNote(const uint16_t Frequency, const uint16_t Amplitude) {
            
            for (int i = 0; i < szTestSound; ++i) {

                rawTestSound[i] = math::fast_sin_bhaskara(360.0*float(Frequency)*float(i)/8000.0) * Amplitude + y0;
            }
        }
    }
}

enum NoteName {

    h = 494,
    b = 466,
    a = 440,
    gH = 415,
    g = 392,
    fH = 370,
    f = 349,
    e = 329,
    dH = 311,
    d = 294,
    cH = 277,
    c = 262
};

class SpeakerNote {

public:

    uint16_t Frequency;
    uint16_t Amplitude;
    uint16_t Duration;

    SpeakerNote() {

        if (!detail::speaker::isInit) {

            mrx::hal::speaker::init();

            mrx::hal::pwm::enablePWMTIM2();

            detail::speaker::isInit = true;
        }
    }

    void setupNote(NoteName note, const uint16_t Amplitude, const uint16_t Duration) {

        this->Frequency = static_cast<uint16_t>(note);
        this->Amplitude = Amplitude;
        
        this->Duration = Duration;

        detail::note::setUpNote(Frequency, Amplitude);
    }

    void play() {
        
        // setupNote(NoteName::a, 2000, 1000);

        // mrx::hal::speaker::startSound(&LaughterSound, 1000);
        mrx::hal::speaker::startSound(&TestSound, this->Duration);
    }

    void stop() {

        mrx::hal::speaker::stopSound();
    }

    bool isNoteEnd() {

        return mrx::hal::speaker::soundController.sound == nullptr;
    }
};