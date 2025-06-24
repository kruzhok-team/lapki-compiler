#include "IRpkg.h"


#include <algorithm>
#include <stdexcept>
#include <string>

#include "Settings.h"

void IRpkg::setWord(uint16_t w16) {
    word16_ = w16;
    bits_count_ = 0;
    for (; w16; w16 /= 2) bits_count_++;
}

// TODO magical numbers: redo with settings v_min v_max
IRpkg::operator std::string() const {
    std::string val;
    auto temp = word16_;
    while (temp) {
        val.push_back(
            (temp % 16 >= 10 ? temp % 16 - 10 + 'A' : '0' + temp % 16));
        temp /= 16;
    }
    std::reverse(val.begin(), val.end());
    return val;
}
