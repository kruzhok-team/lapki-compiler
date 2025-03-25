#pragma once

#include "LED.hpp"

/*
    Pattern service
*/

// Тип Pattern{x} строго АГРЕГАТ (см. функцию Matrix::setRow, Matrix::setCol, Matrix::setPattern(...))
struct Pattern5 {

    uint8_t a1, a2, a3, a4, a5;
};

struct Pattern25 {

    uint8_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35;
};

/*
    Mask service
*/

// Тип операнда
enum class Operand {

    AND,
    OR,
    XOR,
};

namespace detail {

    namespace matrix {

        namespace mask {

            using OpFunc = bool (*)(bool a, bool b);

            namespace func {

                bool AND(const bool a, const bool b) {

                    return a & b;
                }
                
                bool OR(const bool a, const bool b) {

                    return a | b;
                }

                bool XOR(const bool a, const bool b) {

                    return a ^ b;
                }
            }
            
            // Логика по маппингу функтора для операнда
            OpFunc getOpFunc(const Operand op) {

                switch (op) {

                    case Operand::AND:

                        return func::AND;
                    case Operand::OR:
                    
                        return func::OR;
                    case Operand::XOR:

                        return func::XOR;
                }

                return func::AND;
            }
        }
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

        for (uint8_t i(0); i < mrx::hal::matrix::LEDS_COUNT; ++i) {

            detail::matrix::leds[i] = LED(i + 1);
        }

        clear();
    }

    // row in range [0 : mrx::hal::matrix::ROW_SZ]
    // col in range [0 : mrx::hal::matrix::COL_SZ]
    void setPixel(const uint8_t row, const uint8_t col, const uint8_t value) {

        setPixel(row * mrx::hal::matrix::COL_COUNT + col, value);
    }

    // idx == linear index of led
    void setPixel(const uint8_t idx, const uint8_t value) {

        detail::matrix::leds[idx].on(value);
    }

    void setRow(const uint8_t idx, const Pattern5& pattern) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i(0); i < mrx::hal::matrix::COL_COUNT; ++i) {

            setPixel(idx, i, ptrPattern[i]);
        }
    }

    void setCol(const uint8_t idx, const Pattern5& pattern) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);
        
        for (int i(0); i < mrx::hal::matrix::ROW_COUNT; ++i) {

            setPixel(i, idx, ptrPattern[i]);
        }
    }

    void setPattern(const Pattern25& pattern) {

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

    // Далее функции, работающие с масками

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t row, const uint8_t col, const uint8_t value, const Operand op) {

        maskPixel(row * mrx::hal::matrix::COL_COUNT + col, value, op);
    }

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t idx, const uint8_t value, const Operand op) {

        maskPixel(idx, value, detail::matrix::mask::getOpFunc(op));
    }

    void maskRow(const uint8_t idx, const Pattern5& pattern, const Operand op) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i(0); i < mrx::hal::matrix::COL_COUNT; ++i) {

            maskPixel(idx, i, ptrPattern[i], op);
        }
    }

    void maskCol(const uint8_t idx, const Pattern5& pattern, const Operand op) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);
        
        for (int i(0); i < mrx::hal::matrix::ROW_COUNT; ++i) {

            maskPixel(i, idx, ptrPattern[i], op);
        }
    }

    void maskPattern(const Pattern25& pattern, const Operand op) {

        const auto&& func = detail::matrix::mask::getOpFunc(op);
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {

            maskPixel(i, ptrPattern[i], func);
        }
    }

private:

    void maskPixel(const uint8_t idx, const uint8_t value, const detail::matrix::mask::OpFunc func) {

        // logic toggle led for optimize calling from setPattern etc

        const auto&& state = detail::matrix::leds[idx].getState();

        if (func(state, value))
            detail::matrix::leds[idx].on();
        else
            detail::matrix::leds[idx].off();
    }
};