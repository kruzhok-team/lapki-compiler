#pragma once
#include <cstdint>
#include <cstddef>

class Func
{
public:
    static bool withNoise;
    int32_t prev;
    
    void call(int8_t* buf);
};