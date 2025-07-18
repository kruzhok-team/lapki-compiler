#include "LedStrip.h"

#include "Serial.h"
#include "ws2812bTim.h"

int LedStrip::checkNumeration(unsigned int number) {
    if (number > 0 && number <= npx_leds.clr_buf.size()) return number;
    Serial::Printf("wrong numeration, ledstrip number are 1...%d\r\n",
                   npx_leds.clr_buf.size());
    if (number <= 0) return 1;
    if (number > npx_leds.clr_buf.size()) return npx_leds.clr_buf.size();
    return 1;
}

void LedStrip::setColorRGB(int number, uint8_t r, uint8_t g, uint8_t b) {
    number = checkNumeration(number);
    number -= 1;
    npx_leds.clr_buf[number].FromRGB(r, g, b);
    npx_leds.clr_buf[number].ApplyGammaCorrectionRGB();
    npx_leds.SetCurrentColors();
}

// void LedStrip::setColorRGBW(int number, uint8_t r, uint8_t g, uint8_t b,
//                             uint8_t w) {
//     number = checkNumeration(number);
//     number -= 1;
//     npx_leds.clr_buf[number].FromRGB(r, g, b);
//     npx_leds.clr_buf[number].W = w;
//     npx_leds.clr_buf[number].ApplyGammaCorrectionRGBW();
//     npx_leds.SetCurrentColors();
// }

// H: 0...360, S: 0...100, V: 0...100
void LedStrip::setColorHSV(int number, uint16_t H, uint8_t S, uint8_t V) {
    number = checkNumeration(number);
    number -= 1;
    npx_leds.clr_buf[number].FromHSV(H, S, V);
    npx_leds.clr_buf[number].ApplyGammaCorrectionRGB();
    npx_leds.SetCurrentColors();
}

void LedStrip::turnOff(){
    npx_leds.SetAll(clBlack);
    npx_leds.SetCurrentColors();
}