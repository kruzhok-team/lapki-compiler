#pragma once

#include "LED.hpp"
#include "Pattern.hpp"

/*
    Mask service
*/

// Тип операнда
enum Operand {

    AND,
    OR,
    XOR,
};

namespace detail {

    namespace matrixMask {

        bool isInit { false };

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

namespace detail {

    namespace matrixMask {

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

class MatrixMask {

public:

    // ctor
    MatrixMask() {
        
        if (!detail::matrixMask::isInit) {

            for (uint8_t i(0); i < mrx::hal::matrix::LEDS_COUNT; ++i) {

                detail::matrixMask::leds[i] = LED(i + 1);
            }

            detail::matrixMask::isInit = true;
        }
    }

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t row, const uint8_t col, const uint8_t value, const Operand op) {

        maskPixel(row * mrx::hal::matrix::COL_COUNT + col, value, op);
    }

    // Аналогична setPixel с дополнительным аргументом, задающим бинарную маску для установки пикселя
    void maskPixel(const uint8_t idx, const uint8_t value, const Operand op) {

        maskPixel(idx, value, detail::matrixMask::getOpFunc(op));
    }

    void maskRow(const uint8_t idx, const Pattern5& pattern, const Operand op) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i(0); i < mrx::hal::matrix::COL_COUNT; ++i) {

            maskPixel(idx, i, ptrPattern[i], op);
        }
    }

    void maskCol(const uint8_t idx, const Pattern7& pattern, const Operand op) {
        
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);
        
        for (int i(0); i < mrx::hal::matrix::ROW_COUNT; ++i) {

            maskPixel(i, idx, ptrPattern[i], op);
        }
    }

    void maskPattern(const Pattern35& pattern, const Operand op) {

        const auto&& func = detail::matrixMask::getOpFunc(op);
        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < mrx::hal::matrix::LEDS_COUNT; ++i) {

            maskPixel(i, ptrPattern[i], func);
        }
    }

private:

    void maskPixel(const uint8_t idx, const uint8_t value, const detail::matrixMask::OpFunc func) {

        // logic toggle led for optimize calling from setPattern etc

        const auto&& state = detail::matrixMask::leds[idx].getState();

        if (func(state, value))
            detail::matrixMask::leds[idx].on();
        else
            detail::matrixMask::leds[idx].off();
    }
};