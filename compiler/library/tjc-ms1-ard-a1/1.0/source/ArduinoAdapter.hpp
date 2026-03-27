#include "arduino_link.c"

class ArduinoAdapter {
public:
    uint8_t lastByte = 0;
    bool _isQueueOverflow = false;

    ArduinoAdapter() {}

    void init() {
        arduino_link_init();
    }

    bool packetReceived() {
        uint8_t status = arduino_link_recv_byte(&lastByte);
        return status != 0;
    }

    void sendPacket(uint8_t byte) {
        bool status = arduino_link_send_byte(byte);
        if (!status) {
            _isQueueOverflow = false;
            return;
        }
        _isQueueOverflow = true;
    }

    bool isQueueOverflow() {
        return _isQueueOverflow;
    }

};
