#pragma once

#include "MatrixTypes.hpp"

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

                    case Operand::mask_and:

                        return func::AND;
                    case Operand::mask_or:
                    
                        return func::OR;
                    case Operand::mask_xor:

                        return func::XOR;
                }

                return func::AND;
            }
        }
    }
}


// TODO descr
class Matrix {

public:

    // ctor
    Matrix() {
        clear();
    }

    // row in range [0 : details::constant::ROW_SZ]
    // col in range [0 : details::constant::COL_SZ]
    void setPixel(const uint8_t row, const uint8_t col, const uint8_t value) {

        setPixel(row * detail::constants::ROW_SZ + col, value);
    }

    // idx == linear index of led
    void setPixel(const uint8_t idx, const uint8_t value) {

        (void)value;
        detail::service::onLed(detail::leds[idx]);
    }

    void offPixel(const uint8_t row, const uint8_t col) {

        offPixel(row * detail::constants::ROW_SZ + col);
    }

    void offPixel(const uint8_t idx) {

        detail::service::offLed(detail::leds[idx]);
    }

    void setPattern(const Pattern& pattern) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            if (ptrPattern[i] > 0)
                setPixel(i, ptrPattern[i]);
            else {
                offPixel(i);
            }
        }
    }

    void fill(const uint8_t value) {

        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            setPixel(i, value);
        }
    }

    void clear() {
        
        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            offPixel(i);
        }
    }

    // Далее функции, работающие с масками

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t row, const uint8_t col, const uint8_t value, const Operand op) {

        maskPixel(row * detail::constants::ROW_SZ + col, value, op);
    }

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t idx, const uint8_t value, const Operand op) {

        maskPixel(idx, value, detail::matrix::mask::getOpFunc(op));
    }

    void maskRow(const uint8_t idx, const Pattern5& pattern, const Operand op) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i(0); i < detail::constants::COL_SZ; ++i) {

            maskPixel(idx, i, ptrPattern[i], op);
        }
    }

    void maskCol(const uint8_t idx, const Pattern5& pattern, const Operand op) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);
        
        for (int i(0); i < detail::constants::ROW_SZ; ++i) {

            maskPixel(i, idx, ptrPattern[i], op);
        }
    }

    void maskPattern(const Pattern& pattern, const Operand op) {

        const auto&& func = detail::matrix::mask::getOpFunc(op);
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            maskPixel(i, ptrPattern[i], func);
        }
    }

private:

    void maskPixel(const uint8_t idx, const uint8_t value, const detail::matrix::mask::OpFunc func) {

        // logic toggle led for optimize calling from setPattern etc

        const auto state = detail::leds[idx].state;

        if (func(state, value))
            detail::service::onLed(detail::leds[idx]);
        else
            detail::service::offLed(detail::leds[idx]);
    }
};