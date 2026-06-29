#ifndef FUNC_HPP
#define FUNC_HPP

#include <cstdint>
#include <cstddef>

int32_t func_variable(void);

void func_abs(int8_t* data, size_t size);
int32_t func_sum(const int8_t* input, size_t size);

int32_t func_smooth(int32_t value, float coeff, int32_t* prev);
bool func_greater(int32_t a, int32_t b);
#endif