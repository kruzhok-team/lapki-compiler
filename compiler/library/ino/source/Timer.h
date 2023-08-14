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
#include "qhsm.h"
#define NO_PREDELAY 0

class Timer
{
  public:
    Timer(unsigned long interval, QHsm* qhsm);
    Timer(unsigned long prev, unsigned long interval, QHsm* qhsm, char signal[]);

    void reset();
    void disable();
    void enable();
    void check();

    void setInterval(unsigned long interval);

  private:
    bool active;
    unsigned long previous;
    unsigned long interval;

    void (*execute)();

};

#endif
// TIMER_H