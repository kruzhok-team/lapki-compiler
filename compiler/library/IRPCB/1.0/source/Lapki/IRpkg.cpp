#include "IRpkg.h"

#include <stdint-gcc.h>

#include <stdexcept>
#include <string>

#include "Settings.h"
#include <algorithm>

void IRpkg::setWord(uint16_t w16) {
    word16_ = w16;
    bits_count_ = 0;
    for (; w16; w16 /= 2) bits_count_++;
}

// TODO magical numbers: redo with settings v_min v_max
void IRpkg::setPower(int pwr) {
    if (pwr < 0 || pwr > 255) {
        exit(1);
        // throw std::runtime_error(
        //     "setPower: Power is out of limits:\n0 <= power <=255\n");
    }
    power = pwr;
}
// TODO magical numbers: redo with settings v_min v_max
IRpkg::operator std::string() const {
    std::string val;
    auto temp = word16_;
    while (temp) {
        val.push_back((temp % 16 >= 10? temp%16 -10 + 'A': '0' + temp%16));
        temp /= 16;
    }
    std::reverse(val.begin(), val.end());
    return val;
}
