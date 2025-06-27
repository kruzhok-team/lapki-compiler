#ifndef LEDSTRIP_h
#define LEDSTRIP_h

#include "Serial.h"
#include "ws2812bTim.h"
// from InitFunctions.hpp
extern Neopixels npx_leds;

namespace LedStrip {

// 0 ... 255
void setColorRGB(int number, uint8_t r, uint8_t g, uint8_t b);

// 0 ... 255
void setColorRGBW(int number, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

// H: 0...360, S: 0...100, V: 0...100
void setColorHSV(int number, uint16_t H, uint8_t S, uint8_t V);

void setOff();

};  // namespace LedStrip

#endif