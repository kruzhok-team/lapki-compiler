#pragma once
#include <random>


namespace detail {

    namespace random {
        uint32_t seed = mrx::hal::random::mkSeed();
    }
}


// Компонент для генерации псевдо-случайного числа. Seed задается при помощи отсчета времени
class Random {
    // есть эдж кейсы, когда он ломает
    uint32_t abs(int32_t x) {

        if (x < 0)
            return -x;

        return x;
    }
    
public:

    /* Снимаемые значения: знаковое и беззнаковое */
    uint64_t uValue;
    int value;

    Random() {
        srand(detail::random::seed);
    }

    void setSeed(const uint32_t seed) {
        srand(seed);
    }

    // Устанавливает новое случайное значение в value и uValue
    void doRandom() {
        int newVal = rand();
        value = newVal;
        uValue = abs(newVal);

        return;
    }

    // doRandom для заданного диапазона [begin; end)
    void doRangeRandom(const int64_t begin, const int64_t end) {

        doRandom();

        // Для знакового
        if (value < begin || value >= end) {
            // От начала нашего диапазона добавляем допустимые пределы разброса
            value = begin + (abs(value) % (end - begin));
        }

        // Для беззнакового
        if (uValue < begin || uValue >= end) {

            auto _begin = begin;
            if (_begin < 0)
                _begin = 0;

            auto _end = end;
            if (_end < _begin)
                _end = _begin + 1;

            // x - допустимые пределы разброса для случайного значения
            const int x = _end - _begin;
            uValue = begin + (uValue % x);
        }

        return;
    }
};