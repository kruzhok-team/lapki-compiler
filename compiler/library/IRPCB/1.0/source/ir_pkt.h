/*
 * ir_pkt.h
 *
 *  Created on: 13.08.2023
 *      Author: Kreyl
 */

#ifndef IR_PKT_H_
#define IR_PKT_H_

#include "types.h"
#include "shell.h"

int32_t Damage_IdToHits(int32_t damage_id);
RetvValI32 Damage_HitsToId(int32_t hits);

enum class PktType {
    Shot = 0x0000,
    AddHealth = 0x8000,
    AddCartridges = 0x8100,
    NewGame = 0x8305,
};

#pragma pack(push, 1)
class IRPkt {
public:
    uint32_t bits_cnt;
    union {
        uint16_t word16;
        struct { // LSB to MSB
            uint32_t _reserved: 2; // Not used in 14-bit pkt
            uint32_t damage_id: 4;
            uint32_t team_id: 2;
            uint32_t player_id: 7;
            uint32_t zero: 1;
        };
    };

    IRPkt() : bits_cnt(0), word16(0) {}
    IRPkt(uint32_t abit_cnt, uint16_t aword16) : bits_cnt(abit_cnt), word16(aword16) {}

    void Print(Shell *pshell, const char* S) {
        if(bits_cnt == 14 and zero == 0) pshell->Print("%S W16=%04X sz=%u; pl_id %u; tm_id %u; dmg %u\r\n",
                S, word16, bits_cnt, player_id, team_id, damage_id);
        else pshell->Print("%S W16=%04X sz=%u\r\n", S, word16, bits_cnt);
    }

    int32_t GetDamageHits() { return Damage_IdToHits(damage_id); }

    IRPkt& operator =(const IRPkt &right) {
        bits_cnt = right.bits_cnt;
        word16 = right.word16;
        return *this;
    }
};
#pragma pack(pop)

#endif /* IR_PKT_H_ */
