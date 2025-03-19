#pragma once

class Iterator {

    uint16_t from{}, to{}, step{};
    bool isActive{};

public:

    uint16_t index{};

    Iterator() {}

    void start(uint16_t from, uint16_t to, uint16_t step) {

        this->from = from;
        this->to = to;
        this->step = step;

        index = from;
        isActive = true;
    }

    void break_() {

        isActive = false;
    }

    void onIteration() {

        if (isActive) {

            index += step;
            if (index >= to) {
                isActive = false;
            }
        }
    }

    bool onEnd() {

        return isActive;
    }
};