#include "service.h"

const KeyStroke KeyStrokes[]= {
{    TIME_TICK_1M_SIG,         "TIME_TICK_1M",         't'},
{    PILL_ABILITY_SIG,         "PILL_ABILITY",         'p'},
{    LONG_PRESS_THIRD_SIG,     "LONG_PRESS_THIRD",     'l'},
{    TIME_TICK_1S_SIG,         "TIME_TICK_1S",         't'},
{    TERMINATE_SIG,               "TERMINATE",                 0x1B   }

};

unsigned int KeyNumber() {
	return sizeof(KeyStrokes)/sizeof(KeyStrokes[0]);
}
