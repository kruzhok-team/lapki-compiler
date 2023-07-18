#ifndef EVENTSHANDLERS_H_
#define EVENTSHANDLERS_H_

#include "stdio.h"
#include "qhsm.h"
#include <stdbool.h>
#include "oregonPlayer.h"


//logic functions
void UpdateHP(OregonPlayer* me, unsigned int HP);
void Reset(OregonPlayer* me);
void Flash(unsigned int R, unsigned int G, unsigned int B, unsigned int Timeout);
void BeepForPeriod(unsigned int Period);
void SaveHP(unsigned int HP);
void SaveTimerAgony(unsigned int Timer);
void SaveState(unsigned int State);
void ShowCurrentHealth (OregonPlayer* me);
void ShowCurrentHealthGhoul (OregonPlayer* me);
void ClearPill(void);
void PillIndicate(void);


#endif /* EVENTSHANDLERS_H_ */