#pragma once

#include "OverflowHelpers.hpp"


struct MathFloat {
    using AccType = float;
    using ArgType = float;

    static inline bool overflowF = 0;
    static inline bool zeroDivideF = 0;

    static inline AccType value = 0;

    static void sum(const ArgType a, const ArgType b) {

        const auto&& eval = addWithCheckOverflow(a, b);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void dif(const ArgType a, const ArgType b) {

        const auto&& eval = subWithCheckOverflow(a, b);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void mult(const ArgType a, const ArgType b) {

        const auto&& eval = mulWithCheckOverflow(a, b);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void div(const ArgType a, const ArgType b) {

        const auto&& eval = divWithCheckOverflow(a, b);

        overflowF = eval.isError;
        zeroDivideF = equalFloating(b, .0);

        value = eval.eval;
    }

    static void mod(const ArgType a, const ArgType b) {

        const auto&& eval = modWithCheckOverflow(a, b);

        overflowF = eval.isError;
        zeroDivideF = equalFloating(b, .0);

        value = eval.eval;
    }

    // C has macros abs (name conflict)
    static void ABS(const ArgType a) {

        if (a < 0)
            mult(a, -1);
    }

    static void pow(const ArgType a, const ArgType b) {

        const auto&& eval = powWithCheckOverflow(a, b);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void sqrt(const ArgType a) {

        const auto&& eval = sqrtWithCheckOverflow(a);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void log(const ArgType base, const ArgType a) {

        const auto&& eval = logWithCheckOverflow(a, base);

        overflowF = eval.isError;

        value = eval.eval;
    }

    static void applyBasicOperator(const ArgType a, char op, const ArgType b) {
        switch (op) {
            case '+':
                sum(a, b);
                break;
            case '-':
                dif(a, b);
                break;
            case '*':
                mult(a, b);
                break;
            case '/':
                div(a, b);
                break;
            case ':':
                div(a, b);
                break;
        }
    }

    static void reset_state(){
        overflowF = false;
        zeroDivideF = false;
        value = 0;
    }

    // событие на переполнение
    static bool isOverflow() {

        const auto res = overflowF;
        overflowF = false;

        return res;
    }

    // событие на деление на ноль
    static bool isZeroDivision() {

        const auto res = zeroDivideF;
        zeroDivideF = false;

        return res;
    }

};