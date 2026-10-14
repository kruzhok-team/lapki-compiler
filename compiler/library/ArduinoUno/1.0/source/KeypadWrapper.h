#ifndef KEYPAD_WRAPPER_H
#define KEYPAD_WRAPPER_H

#include "Keypad.h"

constexpr uint8_t ROWS_NUMBER = 4;
constexpr uint8_t COLS_NUMBER = 4;

char keys8to1[ROWS_NUMBER][COLS_NUMBER] = {{'1', '2', '3', 'A'},
                                           {'4', '5', '6', 'B'},
                                           {'7', '8', '9', 'C'},
                                           {'*', '0', '#', 'D'}};

// char keys1to8[ROWS_NUMBER][COLS_NUMBER] = {
//   {'1','4','7', '*'},
//   {'2','5','8', '0'},
//   {'3','6','9', '#'},
//   {'A','B','C', 'D'}
// };

struct Pins1to8Container {
    uint8_t pins1to8[8];

    uint8_t operator[](uint8_t k) const { return pins1to8[k]; }
};

// single key handling
class KeypadWrapper {
    uint8_t rows_pins[4];
    uint8_t cols_pins[4];
    Keypad keypad;
    KeyState lastState = IDLE;
    enum FlagMask : uint8_t {
        FkeyPressed = 1 << 0,
        FkeyHeld = 1 << 1,
        Fsymbol_star_hashtag = 1 << 2,
        Fdigit_0 = 1 << 3,
        Fdigit_0_held = 1 << 4,
        Fdigit_1_9 = 1 << 5,
        Fletter_ABCD = 1 << 6
    };
    uint8_t flags = 0;
    static constexpr uint8_t pressedFlagsMask =
        FkeyPressed | Fsymbol_star_hashtag | Fdigit_0 | Fdigit_1_9 | Fletter_ABCD;
    static constexpr uint8_t heldFlagsMask = FkeyHeld | Fdigit_0_held;

   public:
    char lastSymbol = 0;

    KeypadWrapper(Pins1to8Container pins1to8)
        : keypad(makeKeymap(keys8to1), fillRows(pins1to8), fillCols(pins1to8),
                 ROWS_NUMBER, COLS_NUMBER) {
        keypad.addEventListener(this, [](void* context, KeypadEvent key) {
            static_cast<KeypadWrapper*>(context)->keypadEvent(key);
        });
    }

    bool keyPressed() {
        bool temp = flags & FkeyPressed;
        flags = flags & ~FkeyPressed;
        return temp;
    }

    bool keyHeld() {
        bool temp = flags & FkeyHeld;
        flags = flags & ~FkeyHeld;
        return temp;
    }

    bool symbol_star_hashtag() {
        if (!(flags & Fsymbol_star_hashtag)) return 0;
        if (lastSymbol == keys8to1[3][0] || lastSymbol == keys8to1[3][2]) {
            flags = flags & ~Fsymbol_star_hashtag;
            return 1;
        }
        return 0;
    }

    bool digit_0() {
        if (!(flags & Fdigit_0)) return 0;
        if (lastSymbol == keys8to1[3][1]) {
            flags = flags & ~Fdigit_0;
            return 1;
        }
        return 0;
    }

    bool digit_0_held() {
        if (!(flags & Fdigit_0_held)) return 0;
        if (lastSymbol == keys8to1[3][1]) {
            flags = flags & ~Fdigit_0_held;
            return 1;
        }
        return 0;
    }

    bool digit_1_9() {
        if (!(flags & Fdigit_1_9)) return 0;
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++)
                if (lastSymbol == keys8to1[i][j]) {
                    flags = flags & ~Fdigit_1_9;
                    return 1;
                }
        }
        return 0;
    }

    bool letter_ABCD() {
        if (!(flags &  Fletter_ABCD)) return 0;
        if (lastSymbol == keys8to1[0][3] || lastSymbol == keys8to1[1][3] ||
            lastSymbol == keys8to1[2][3] || lastSymbol == keys8to1[3][3]) {
            flags = flags &  ~Fletter_ABCD;
            return 1;
        }
        return 0;
    }

    // loopAction
    void updateSymbol() { keypad.getKeys(); }

    void change_key(uint8_t r, uint8_t c, char newKey) {
        keys8to1[r][c] = newKey;
    }

   private:
    uint8_t* fillRows(const Pins1to8Container& pins1to8) {
        for (int i = 0; i < 4; i++) {
            rows_pins[i] = pins1to8[7 - i];
        }
        return rows_pins;
    }

    uint8_t* fillCols(const Pins1to8Container& pins1to8) {
        for (int i = 0; i < 4; i++) {
            cols_pins[i] = pins1to8[3 - i];
        }
        return cols_pins;
    }

    void keypadEvent(KeypadEvent key) {
        flags = 0;
        if (lastState != HOLD) {
            switch (keypad.getState()) {
                case RELEASED:
                    lastSymbol = key;
                    flags = pressedFlagsMask;
                    break;
                case HOLD:
                    lastSymbol = key;
                    flags = heldFlagsMask;
                    break;
            }
        }
        lastState = keypad.getState();
    }
};

#endif