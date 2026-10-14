#pragma once

#include <stdint.h>
struct VarInt {
    using AccType = int32_t;
    using ArgType = int32_t;
    
    AccType value;

    void set(const ArgType value) {
        this->value = value;
    }

    // TODO может добавить isInf / isNan?
};