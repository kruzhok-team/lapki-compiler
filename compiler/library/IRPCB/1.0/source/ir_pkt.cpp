/*
 * ir_pkt.cpp
 *
 *  Created on: 1 июл. 2024 г.
 *      Author: Kreyl
 */

#include "ir_pkt.h"

static const int32_t kDamTableSz = 16;

static const int32_t kDamageTable[kDamTableSz] = {
        1,2,4,5,7,10,15,17,20,25,30,35,40,50,75,100
};

int32_t Damage_IdToHits(int32_t damage_id) {
    return kDamageTable[damage_id];
}

RetvValI32 Damage_HitsToId(int32_t hits) {
    RetvValI32 r{retv::BadValue};
    for(int32_t i=0; i<kDamTableSz; i++) {
        if(hits == kDamageTable[i]) {
            r.v = i;
            r.rslt = retv::Ok;
            break;
        }
    }
    return r;
}
