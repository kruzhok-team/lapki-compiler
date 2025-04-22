#pragma once

#include "SpeakerSound.hpp"
#include "CommonSound.hpp"

namespace detail {

    namespace speaker {
        // defined in SpeakerSound
        // bool isInit = false;
    }

    namespace math {

        namespace sin {

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
    }

    namespace note {

        float V = 440;
        const uint16_t y0 = 2047;
        const float pi = 3.1415926;

        void setUpNote(const uint16_t Frequency, const uint16_t Amplitude) {
            
            for (int i = 0; i < szTestSound; ++i) {

                rawTestSound[i] = y0 + Amplitude * math::sin(2*pi*V*i/8000);
            }
        }
    }
}

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

    void setupSound(const uint16_t Frequency, const uint16_t Amplitude, const uint16_t Duration) {

        this->Frequency = Frequency;
        this->Amplitude = Amplitude;
        
        this->Duration = Duration;

        detail::note::setUpNote(Frequency, Amplitude);
    }

    void play() {
        
        setupSound(8000, 1000, 444);

        mrx::hal::speaker::startSound(&TestSound, this->Duration);
    }

    void stop() {

        mrx::hal::speaker::stopSound();
    }

    bool isNoteEnd() {

        return mrx::hal::speaker::soundController.sound == nullptr;
    }
};