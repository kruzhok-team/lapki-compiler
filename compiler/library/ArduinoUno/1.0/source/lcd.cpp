#include "Lcd.h"

#include <ctype.h>

void Lcd::init() {
    lcd_.begin();
    lcd_.backlight();
}

size_t Lcd::write(uint8_t c) {
    return lcd_.write(c);
};

void Lcd::reset() {
    lcd_.home();
    lcd_.clear();
}

void Lcd::setCursor(uint8_t col, uint8_t row){
    lcd_.setCursor(col, row); //verification is inside
}

void Lcd::backlight() {
    lcd_.backlight();
}

void Lcd::noBacklight() {
    lcd_.noBacklight();
}

