#include "AustereSerialMaster.h"

static const uint8_t PIN_MISO = 2;
static const uint8_t PIN_MOSI = 3;
static const uint8_t PIN_CLK  = 4;
static const uint8_t PIN_CS   = 5;
static const uint8_t PIN_IRQ  = 6;

AustereSerialMaster link(PIN_MISO, PIN_MOSI, PIN_CLK, PIN_CS, PIN_IRQ);

class TjcSerial {
public:

uint8_t lastByte;

TjcSerial() {};

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
