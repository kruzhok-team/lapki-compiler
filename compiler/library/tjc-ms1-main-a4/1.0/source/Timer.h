/* $Id: Timer.h 1198 2011-06-14 21:08:27Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Provides an easy way of triggering functions at a set interval.
|| |
|| | Wiring Cross-platform Library
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef TIMER_H
#define TIMER_H

class Timer {
    
public:
    Timer() {

        _active = false;
        _previous = 0;
    }

    void reset() {

        _previous = millis();
    }

    void disable() {

        _active = false;
    }

    void enable() {

        _active = true;
    }

    bool timeout() {

        if (_active && (millis() - _previous >= _interval))
        {
            _previous = millis();
            return true;
        }
        return false;
    }

    void setInterval(unsigned long interval) {

        _interval = interval;
    }

    void start(unsigned long interval) {

        setInterval(interval);
        reset();
        enable();
    }

private:
    bool _active;
    unsigned long _previous;
    unsigned long _interval;
    bool _oneshot;
};

#endif
// TIMER_H