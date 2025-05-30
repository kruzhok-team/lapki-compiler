#ifndef IRPKG_HPP
#define IRPKG_HPP
// #include <stdint-gcc.h>
#include <string>

#include "Settings.h"

class IRpkg {
    uint16_t word16_ = 0;
    uint32_t bits_count_ = 0;
    int32_t power = *settings.ir_tx_pwr;

   public:
    IRpkg& self;
    const uint16_t& word16 = word16_;
    const uint32_t& bits_count = bits_count_;

    IRpkg() : self(*this) {};

    IRpkg& operator=(const IRpkg& other) {
        if (this != &other) {
            word16_ = other.word16_;
            bits_count_ = other.bits_count_;
            power = other.power;
        }
        return *this;
    }

    IRpkg(const IRpkg& other) : self(*this) { *this = other; }

    void setWord(uint16_t w16);

    
    void setPower(int pwr);

    inline void setPkg(IRpkg other) { *this = other; }

    inline void set(uint16_t w16, int pwr) {
        setWord(w16);
        setPower(pwr);
    }
    operator std::string() const;
};

#endif  // IRPKG_HPP