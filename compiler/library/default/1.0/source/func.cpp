#include "func.hpp"

int32_t func_variable(void) {
    return 0;
}

void func_abs(int8_t* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (data[i] < 0) {
            data[i] = -data[i];
        }
    }
}

int32_t func_sum(const int8_t* input, size_t size) {
    int32_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += input[i];
    }
    return sum;
}

int32_t func_smooth(int32_t value, float coeff, int32_t* prev) {
    // Сохраняем старое значение, вычисляем новое, записываем в prev
    int32_t old_prev = *prev;
    int32_t result = (int32_t)(coeff * value + (1.0f - coeff) * old_prev);
    *prev = result;
    return result;
}

bool func_greater(int32_t a, int32_t b) {
    return a > b;
}