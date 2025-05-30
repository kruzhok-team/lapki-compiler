#ifndef TIMER_H
#define TIMER_H
extern "C++" {
#include "yartos.h"
constexpr systime_t tickDuration = 200;

// everything in ms;

class Timer {
    systime_t start_;
    systime_t previous_;
    systime_t prevTick_;
    systime_t interval_;
    bool active_;

   public:
    // обновляется в суперцикле через updateDifference
    systime_t difference;

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