/*
 * ir.h
 *
 *  Created on: 04.07.2013
 *      Author: kreyl
 */

#pragma once

#include "gd_lib.h"
#include "board.h"

#define IR_TX_ENABLED   TRUE
#define IR_RX_ENABLED   TRUE

// Delays, uS
namespace kIr {
inline const uint32_t Header_us = 2400UL;
inline const uint32_t Space_us = 600UL;
inline const uint32_t Zero_us = 600UL;
inline const uint32_t One_us = 1200UL;
inline const uint32_t PauseAfter_us = 2400UL; // Pause after pkt transmission
inline const uint32_t InterBitTimeot_us = 900UL; // For reception: after rising edge, wait falling one for this time. Ideally, pause is 600us - so let's give it a 300us chance
} // namespace

#if IR_TX_ENABLED // ========================== IR TX ==========================
#define IR_MAX_PWR          255     // Top DAC value

namespace IRLed {
    void Init();
    void SetCarrierFreq(uint32_t carrier_freq_Hz);
    void TransmitWord(uint16_t data, int32_t bit_cnt, uint8_t power, ftVoidVoid callbackI);
    void ResetI();
} // namespace
#endif

#if IR_RX_ENABLED // ========================== IR RX ==========================
namespace IRRcvr {
    void Init();
    extern ftVoidU8U16 callbackI;
} // namespace
#endif
