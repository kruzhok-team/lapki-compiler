#pragma once

namespace detail {

    namespace random {
                
        const uint32_t RAND_A { 1103515245 };
        const uint32_t RAND_C { 12345 };
        const uint32_t RAND_M { 2147483648 };

        bool isSeeded { false };

        uint32_t seed{};

        int64_t mapRandom(int64_t x, int64_t in_min, int64_t in_max, int64_t out_min, int64_t out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        uint32_t random() {

            if (!isSeeded) {

                seed = mrx::hal::random::mkSeed();
                isSeeded = true;
            }

            seed = ( RAND_A * seed + RAND_C ) % RAND_M;

            return seed;
        }
    }
}

// Компонент для генерации псевдо-случайного числа. Seed задается при помощи отсчета времени
class Random {
    
public:

    /* Снимаемые значения: знаковое и беззнаковое */
    uint32_t uValue;
    int32_t value;

    Random() {}

    void setSeed(const uint32_t seed) {
        
        detail::random::seed = seed;
        detail::random::isSeeded = true;
    }

    // Устанавливает новое случайное значение в value и uValue
    void doRandom() {

        const auto oldRandomValue = detail::random::seed;
        const auto randomValue = detail::random::random();

        // put to value fields
        value = randomValue;
        uValue = randomValue;

        // random sign for value
        if ((oldRandomValue &1) == 0) {
            value = - value;
        }

        return;
    }

    // doRandom для заданного диапазона [begin; end)
    void doRangeRandom(const int64_t begin, const int64_t end) {

        doRandom();

        // Для знакового
        if (value < begin || value >= end)
            value = detail::random::mapRandom(value, -2147483648, 2147483647, begin, end);

        // Для беззнакового
        if (uValue < begin || uValue >= end) {

            auto _begin = begin;
            if (_begin < 0)
                _begin = 0;

            auto _end = end;
            if (_end < _begin)
                _end = _begin + 1;

            uValue = detail::random::mapRandom(uValue, 0, 4294967295, _begin, _end);
        }

        return;
    }
};