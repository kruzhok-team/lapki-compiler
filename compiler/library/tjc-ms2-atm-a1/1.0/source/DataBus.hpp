#pragma once
#ifdef ATM_SIMPLE_BUS
#error "SimpleBus and DataBus share USART1; choose one component type"
#endif
#define ATM_DATA_BUS
#include "AtmUart.hpp"

class DataBus {
    uint32_t baudrate;
public:
    uint8_t lastByte = 0;
    explicit DataBus(uint32_t baudrate = 9600) : baudrate(baudrate) {}
    void begin() { atm::uart::begin(baudrate); }
    void sendByte(uint8_t value) { atm::uart::send(&value, 1); }
    bool isByteReceived() { return atm::uart::pop(lastByte); }
};
