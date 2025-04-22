#pragma once

#include "CommonSound.hpp"

// Raw structures and Sound structure
// sound data and size -> Sound


// 1
// test sound
// Используется для Пищалка-тембр
const uint32_t szTestSound = 200;
static uint16_t rawTestSound[szTestSound] = { 0 };

Sound TestSound{ rawTestSound, szTestSound, 1 };

// 2
// sine sound
const uint32_t szSineSound = 120;
static uint16_t rawSineSound[szSineSound] = { 2048,2155,2262,2368,2474,2578,2681,2782,2881,2978,3072,3163,3252,3337,3418,3496,3570,3640,3705,3766,3822,3873,3919,3960,3996,4026,4051,4071,4085,4093,4096,4093,4085,4071,4051,4026,3996,3960,3919,3873,3822,3766,3705,3640,3570,3496,3418,3337,3252,3163,3072,2978,2881,2782,2681,2578,2474,2368,2262,2155,2048,1941,1834,1728,1622,1518,1415,1314,1215,1118,1024,933,844,759,678,600,526,456,391,330,274,223,177,136,100,70,45,25,11,3,0,3,11,25,45,70,100,136,177,223,274,330,391,456,526,600,678,759,844,933,1024,1118,1215,1314,1415,1518,1622,1728,1834,1941 };

Sound SineSound{ rawSineSound, szSineSound, 4 };