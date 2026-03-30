#include "AustereSerialMaster.h"

// static const uint8_t PIN_MISO = 2;
// static const uint8_t PIN_MOSI = 3;
// static const uint8_t PIN_CLK  = 4;
// static const uint8_t PIN_CS   = 5;
// static const uint8_t PIN_IRQ  = 6;


class TjcSerial {
public:

uint8_t lastByte;
uint8_t PIN_MISO;
uint8_t PIN_MOSI;
uint8_t PIN_CLK;
uint8_t PIN_CS;
uint8_t PIN_IRQ;

AustereSerialMaster link { 5, 4, 3, 2, 1 };

TjcSerial(uint8_t _miso, uint8_t _mosi, uint8_t _clk, uint8_t _cs, uint8_t _irq) {
    PIN_MISO = _miso;
    PIN_MOSI = _mosi;
    PIN_CLK = _clk;
    PIN_CS = _cs;
    PIN_IRQ = _irq;
    link = AustereSerialMaster(PIN_MISO, PIN_MOSI, PIN_CLK, PIN_CS, PIN_IRQ);
};

void setup() {
    link.setup();
}

void sendByte(uint8_t byte) {
    link.sendByte(byte);
}

bool isByteReceived() {
    bool status = link.recvByte(&lastByte);

    return status;
}
};
