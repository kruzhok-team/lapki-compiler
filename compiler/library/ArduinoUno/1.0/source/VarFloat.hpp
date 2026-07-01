#pragma once

struct VarFloat {
    using AccType = float;
    using ArgType = float;
    
    AccType value;

    void set(const ArgType value) {
        this->value = value;
    }

    // TODO может добавить isInf / isNan?
};