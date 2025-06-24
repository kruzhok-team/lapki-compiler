#ifndef IRPKG_HPP
#define IRPKG_HPP
// #include <stdint-gcc.h>
#include <string>

class IRpkg {
    uint8_t bits_count_ = 0;
    uint16_t word16_ = 0;

   public:
    IRpkg& self;
    const uint8_t& bits_count = bits_count_;
    const uint16_t& word16 = word16_;

    IRpkg() : self(*this) {};

    IRpkg& operator=(const IRpkg& other) {
        if (this != &other) {
            word16_ = other.word16_;
            bits_count_ = other.bits_count_;
        }
        return *this;
    }

    IRpkg(const IRpkg& other) : self(*this) { *this = other; }

    void setWord(uint16_t w16);

    inline void setPkg(IRpkg other) { *this = other; }

    ///////////
    inline void set(uint8_t bts_cnt, uint16_t w16) {
        word16_ = w16;
        bits_count_ = bts_cnt;
    }

    bool operator==(const IRpkg& other) {
        return this->bits_count == other.bits_count &&
               this->word16 == other.word16;
    }

    bool operator!=(const IRpkg& other) { return !(*this == other); }

    operator std::string() const;
};

#endif  // IRPKG_HPP