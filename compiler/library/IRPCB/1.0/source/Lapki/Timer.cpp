#include "Timer.h"

// template <typename T>
// T max(T a, T b) {
//     return (a > b ? a : b);
// }

Timer::Timer()
    : start_(Sys::GetSysTime()),
      previous_(Sys::GetSysTime()),
      prevTick_(Sys::GetSysTime()),
      interval_(0),
      active_(false) {}

void Timer::reset() { previous_ = Sys::GetSysTime(); }

void Timer::pause() {
    interval_ = difference;
    active_ = false;
}

void Timer::proceed() {
    previous_ = Sys::GetSysTime();
    active_ = true;
}

bool Timer::timeout() { return difference == 0; }

void Timer::setInterval(unsigned long interval) { interval_ = interval; }

void Timer::start(unsigned long interval) {
    setInterval(interval);
    reset();
    proceed();
    difference = interval;
}

bool Timer::tick() {
    auto current = Sys::GetSysTime();
    if (current - prevTick_ >= tickDuration) {
        prevTick_ = current;
        return true;
    }
    return false;
}

void Timer::updateDifference() {
    if (active_)
        difference = (interval_ >= (Sys::GetSysTime() - previous_)
                          ? interval_ - (Sys::GetSysTime() - previous_)
                          : 0);
}

