#include "bt.c"
#include <cstdint>

class BluetoothByte {
public:
    uint8_t data = 0;
    // char* data = (char*)answer;

    BluetoothByte() {}

    void init() {
        initBT();
    }

    void sendByte(uint8_t byte) {
        // putString("this is a test\r\n");
        putChar(byte);
    }

    bool isByteReceived() {
        if (gotAnswer) {
            data = answer[0];
            gotAnswer = false;
            return true;
        }
        return gotAnswer;
    }
};
