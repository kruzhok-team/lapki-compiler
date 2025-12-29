#ifndef LEDSTRIP_h
#define LEDSTRIP_h

#include "Serial.h"
#include "ws2812bTim.h"
#include "board.h"
#include "color.h"
// from InitFunctions.hpp
class LedStrip {
    NpxParams params;
    Neopixels npx_leds;
    int checkNumeration(unsigned int number);
   public:
    const uint16_t led_number;
    LedStrip(uint32_t number)
        : params({NPX_PARAMS, NPX_DMA, number, NpxParams::ClrType::RGB}), npx_leds(&params), led_number(number) {
        npx_leds.Init();
        npx_leds.SetAll(clBlack);
        npx_leds.SetCurrentColors();
    }
    // 0 ... 255
    void setColorRGB(int number, uint8_t r, uint8_t g, uint8_t b);

    // 0 ... 255
    // void setColorRGBW(int number, uint8_t r, uint8_t g, uint8_t b, uint8_t
    // w);

    // H: 0...360, S: 0...100, V: 0...100
    void setColorHSV(int number, uint16_t H, uint8_t S, uint8_t V);

    void turnOff();

};  // namespace LedStrip

#endif