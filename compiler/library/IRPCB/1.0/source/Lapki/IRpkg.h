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

    IRpkg(const uint16_t w16) : IRpkg() {
        setWord(w16);
    }

    IRpkg(const IRpkg& other) : self(*this) { *this = other; }

    IRpkg& operator=(const IRpkg& other) {
        if (this != &other) {
            word16_ = other.word16_;
            bits_count_ = other.bits_count_;
        }
        return *this;
    }

    //api
    void setWord(const uint16_t w16);

    //api
    inline void setPkg(const IRpkg& other) { *this = other; }

    inline void set(const uint8_t bts_cnt, const uint16_t w16) {
        word16_ = w16;
        bits_count_ = bts_cnt;
    }    

    operator std::string() const;
};


inline bool operator==(const IRpkg& a, const IRpkg& b) {
        return a.bits_count == b.bits_count &&
               a.word16 == b.word16;
    }

inline bool operator!=(const IRpkg& a, const IRpkg& b) { return !(a == b); }
#endif  // IRPKG_HPP