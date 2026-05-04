#ifndef LCD_H
#define LCD_H

#include <Print.h>

#include "LCD_I2C.h"

class Lcd : public Print {
    LCD_I2C lcd_;
    uint8_t cols_;
    uint8_t rows_;
    bool isOverflowed_ = 0;
    uint8_t emptyCount_;

   public:
   const uint8_t& emptyCount = emptyCount_;
    Lcd(uint8_t address, uint8_t cols = 16, uint8_t rows = 2)
        : lcd_(address, cols, rows),
          cols_(cols),
          rows_(rows),
          emptyCount_(rows * cols) {}

    void init();

    virtual size_t write(uint8_t c) override;

    void reset();

    void setCursor(uint8_t col, uint8_t row);

    void backlight();

    void noBacklight();

};

#endif