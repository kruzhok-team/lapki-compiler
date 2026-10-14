#include "ClassFunc.hpp"
#include "func.hpp"

bool Func::withNoise = false;

void Func::call(int8_t* data) {
    func_abs(data, 60);
    int32_t energy = func_sum(data, 60);
    int32_t smoothed = func_smooth(energy, 0.4f, &prev);
    withNoise = func_greater(smoothed, 4000);
}