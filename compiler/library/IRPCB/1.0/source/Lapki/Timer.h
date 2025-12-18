#ifndef TIMER_H
#define TIMER_H
extern "C++" {
#include "yartos.h"

typedef uint32_t timer_t;

constexpr timer_t tickDuration = 200;

// everything in ms;

class Timer {
    timer_t start_;
    timer_t previous_;
    timer_t prevTick_;
    timer_t interval_;
    bool active_;

   public:
    // обновляется в суперцикле через updateDifference
    timer_t difference = 0;

    Timer();

    void reset();

    void pause();

    void proceed();

    bool timeout();

    void setInterval(unsigned long interval);

    void start(unsigned long interval);

    bool tick();

    void updateDifference();

    static void delay(int ms);
};
}

#endif