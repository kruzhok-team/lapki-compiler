#ifndef COUNTER_H
#define COUNTER_H

class Counter {
   public:
    int value;
    Counter() { value = 0; }
    void add(int value) { this->value += value; }
    void sub(int value) { this->value -= value; }
    void set(int value) { this->value = value; }
    void div(int value) {
        if (value == 0) return;
        this->value /= value;
    }
    void mul(int value) { this->value *= value; }  // this->value — int
    void reset() { this->value = 0; }
};

#endif