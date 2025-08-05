#pragma once

#include "LED.hpp"
#include "Pattern.hpp"

namespace detail {

    namespace matrix {

        bool isInit { false };
    }
}

namespace detail {

    namespace matrix {

        LED leds[mrx::hal::matrix::LEDS_COUNT] {
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
            LED(-1), LED(-1), LED(-1), LED(-1), LED(-1),
        };
    }
}

// Класс Матрица предоставляет интерфейс для компонента 'Matrix' в Lapki Ide, и позволяет пользователю
// удобно манипулировать светодиодами на матрице
class Matrix {

public:

    // ctor
    Matrix() {
        
        if (!detail::matrix::isInit) {

            for (uint8_t i(0); i < mrx::hal::matrix::LEDS_COUNT; ++i) {

                detail::matrix::leds[i] = LED(i + 1);
            }

            clear();

            detail::matrix::isInit = true;
        }
    }

    // row in range [0 : mrx::hal::matrix::ROW_SZ]
    // col in range [0 : mrx::hal::matrix::COL_SZ]
    void setPixel(const uint8_t row, const uint8_t col, const uint8_t value) {

        setPixel(row * mrx::hal::matrix::COL_COUNT + col, value);
    }

    // idx == linear index of led
    void setPixel(const uint8_t idx, const uint8_t value = 100) {

        detail::matrix::leds[idx].on(value);
    }

    void setRow(const uint8_t idx, const Pattern5& pattern) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i(0); i < mrx::hal::matrix::COL_COUNT; ++i) {

            setPixel(idx, i, ptrPattern[i]);
        }
    }

    void changePatternBright(const uint8_t value) {
        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {
            if (detail::matrix::leds[i].value > 0) {
                setPixel(i, value);
            }
        }
    }

    void setCol(const uint8_t idx, const Pattern7& pattern) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);
        
        for (int i(0); i < mrx::hal::matrix::ROW_COUNT; ++i) {

            setPixel(i, idx, ptrPattern[i]);
        }
    }

    void setPattern(const Pattern35& pattern) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {
            setPixel(i, ptrPattern[i]);
        }
    }

    void setPatternByStep(const Pattern35& pattern) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {
            
            setPixel(i, ptrPattern[i]);
        }
    }

    void fill(const uint8_t value) {

        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {

            setPixel(i, value);
        }
    }

    void clear() {
        
        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {

            setPixel(i, 0);
        }
    }
};