#pragma once
#include <stdint.h>

namespace atm {
inline uint8_t crc8(const uint8_t* data, uint8_t size) {
    uint8_t crc = 0xff;
    for (uint8_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = crc & 0x80 ? uint8_t((crc << 1) ^ 0x31) : uint8_t(crc << 1);
    }
    return crc;
}
inline uint16_t word(const uint8_t* bytes) {
    return (uint16_t(bytes[0]) << 8) | bytes[1];
}
inline bool crcPair(const uint8_t* bytes) {
    return crc8(bytes, 2) == bytes[2] && crc8(bytes + 3, 2) == bytes[5];
}
inline int32_t rounded(int32_t numerator, int32_t denominator) {
    return (numerator + (numerator < 0 ? -denominator / 2 : denominator / 2)) / denominator;
}
inline int32_t temperature(uint16_t raw) {
    return rounded(17500 * int32_t(raw) - 4500 * 65535, 65535);
}
inline uint32_t low20(const uint8_t* bytes) {
    return (uint32_t(bytes[0] & 0x0f) << 16) | (uint32_t(bytes[1]) << 8) | bytes[2];
}
inline int32_t signed20(const uint8_t* bytes) {
    const uint32_t raw = low20(bytes);
    return raw & 0x80000 ? int32_t(raw) - 0x100000 : int32_t(raw);
}
inline int32_t signed24(const uint8_t* bytes) {
    const uint32_t raw = (uint32_t(bytes[0]) << 16) | (uint32_t(bytes[1]) << 8) | bytes[2];
    return raw & 0x800000 ? int32_t(raw) - 0x1000000 : int32_t(raw);
}
}
