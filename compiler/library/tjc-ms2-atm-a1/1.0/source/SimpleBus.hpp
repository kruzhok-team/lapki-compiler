#pragma once
#ifdef ATM_DATA_BUS
#error "SimpleBus and DataBus share USART1; choose one component type"
#endif
#define ATM_SIMPLE_BUS
#include "AtmUart.hpp"

class SimpleBus {
    uint8_t frame[5] = {}, size = 0;
    static uint8_t hex(uint8_t value) { return value < 10 ? '0' + value : 'A' + value - 10; }
public:
    uint8_t myAddress = 0, lastData = 0;
    void begin() { atm::uart::begin(9600); size = 0; }
    void setAddress(uint8_t address) { myAddress = address; }
    void sendPacket(uint8_t address, uint8_t data) {
        const uint8_t packet[] = {13, hex(address >> 4), hex(address & 15), data, 10};
        atm::uart::send(packet, 5);
    }
    bool packetReceived() {
        uint8_t value;
        while (atm::uart::pop(value)) {
            frame[size++] = value;
            if (size < 5) continue;
            if (frame[0] == 13 && frame[4] == 10) {
                size = 0;
                if (frame[1] == hex(myAddress >> 4) && frame[2] == hex(myAddress & 15)) {
                    lastData = frame[3];
                    return true;
                }
            } else {
                for (uint8_t i = 0; i < 4; ++i) frame[i] = frame[i + 1];
                size = 4;
            }
        }
        return false;
    }
};
