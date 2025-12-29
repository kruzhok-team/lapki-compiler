/*
 * yartos_cfg.h
 *
 *  Created on: 18 июл. 2023 г.
 *      Author: laurelindo
 */

#ifndef YARTOS_YARTOS_CFG_H_
#define YARTOS_YARTOS_CFG_H_

// System timer resolution: 16 or 32 bits
#define SYS_TIM_RESOLUTION      16

// Frequency of the system timer that drives the system ticks. This setting also defines the system tick time unit.
#define SYS_TIM_FREQUENCY       1000

// Time delta constant for the tick-less mode
// This value represents the minimum number of ticks that is safe to specify in a timeout directive.
//  The value one is not valid, timeouts are rounded up to this value.
#define SYS_ST_TIMEDELTA        2

// Debug check
#define SYS_DBG_CHECK    TRUE


#endif /* YARTOS_YARTOS_CFG_H_ */
