#pragma once

#include "CommonSound.hpp"

// Raw structures and Sound structure
// sound data and size -> Sound


// 1
// test sound
const uint32_t szTestSound = 24;
static uint16_t rawTestSound[szTestSound] = { 2048, 2578, 3072, 3496, 3821, 4026,
                                   4096, 4026, 3821, 3496, 3072, 2578,
                                   2048, 1517, 1023, 599, 274, 69, 0,
                                   69, 274, 599, 1023, 1517 };

Sound TestSound{ rawTestSound, szTestSound };