/* $Id: Timer.cpp 1198 2011-06-14 21:08:27Z bhagman $
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

#include "Timer.h"
/*
|| @constructor
|| | Initialize the Timer object and activate it
|| #
||
|| @parameter intervl  the interval to call the function
|| @parameter function the funtion to call each interval
*/
Timer::Timer(unsigned long intervl, void (*function)())
{
  active = true;
  previous = 0;
  interval = intervl;
  execute = function;
}

/*
|| @constructor
|| | Initialize the Timer object and activate it
|| #
||
|| @parameter prev     the wait time, before starting the interval
|| @parameter intervl  the interval to call the function
|| @parameter function the funtion to call each interval
*/
Timer::Timer(unsigned long prev, unsigned long intervl, void (*function)())
{
  active = true;
  previous = prev;
  interval = intervl;
  execute = function;
}

/*
|| @description
|| | Reset the interval timing
|| #
*/
void Timer::reset()
{
  previous = millis();
}

/*
|| @description
|| | Disable the calling of this Timer
|| #
*/
void Timer::disable()
{
  active = false;
}

/*
|| @description
|| | Enable the calling of this Timer
|| #
*/
void Timer::enable()
{
  active = true;
}

/*
|| @description
|| | Check if it is time for this Timer to call the function
|| #
*/
void Timer::check()
{
  if (active && (millis() - previous >= interval))
  {
    previous = millis();
    execute();
  }
}

/*
|| @description
|| | Set the interval of this Timer
|| #
||
|| @parameter intervl the interval to call the function
*/
void Timer::setInterval(unsigned long intervl)
{
  interval = intervl;
}