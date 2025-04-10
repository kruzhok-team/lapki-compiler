#pragma once

#include "CommonNote.hpp"

// Raw structures and Note structure
// Note data and size -> Note


// 1
// test note
const uint32_t szTestNote = 24;
static uint16_t rawTestNote[szTestNote] = { 2048, 2578, 3072, 3496, 3821, 4026,
                                   4096, 4026, 3821, 3496, 3072, 2578,
                                   2048, 1517, 1023, 599, 274, 69, 0,
                                   69, 274, 599, 1023, 1517 };

Note TestNote{ rawTestNote, szTestNote, 1 };