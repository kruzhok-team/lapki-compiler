/*
 * app.h
 *
 *  Created on: 14.08.2023
 *      Author: Kreyl
 */

#ifndef APP_H_
#define APP_H_

#include <inttypes.h>

extern int32_t hit_cnt, rounds_cnt, magazines_cnt;

void AppInit();
void SetInputs(uint32_t ainputs[2]);
void Reset(bool quiet = false);

void FireSingleShot();
void FireBurst();
void StopFire();

void IrRxCallbackI(uint8_t bit_cnt, uint16_t rcvd);

#endif /* APP_H_ */
