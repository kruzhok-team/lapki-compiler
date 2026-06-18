#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include <errno.h>

#define CHAR_BUFFER_SIZE 32
#define FLOAT_DIGITS_AFTER_DECIMAL 3

class CharBuffer {
   public:
    char buf[CHAR_BUFFER_SIZE]{};
    bool isBufferOverflowF = 0;
    bool isErrorF = 0;

    void appendChar(char c) {
        uint8_t len = strlen(buf);
        if(len + 1 >= CHAR_BUFFER_SIZE) {
            isBufferOverflowF = 1;
            isErrorF = 1;
            return;
        }
        buf[len] = c;
        buf[len + 1] = '\0';
    }

    void reset(){
        buf[0] = '\0';
        isErrorF = 0;
        isBufferOverflowF = 0;
    }

    void appendBuffer(char* buf2){
        if(strlen(buf) + strlen(buf2) + 1 >=CHAR_BUFFER_SIZE){
            isErrorF = 1;
            isBufferOverflowF = 1;
            return;
        }
        strcat(buf, buf2);
    }

    void popChars(uint8_t n) {
        uint8_t len = strlen(buf);
        if (n > len) {
            isErrorF = 1;
            return;
        }
        buf[len - n] = '\0';
    }

    void toNumber(float& num){
        char* end = NULL;
        num = strtod(buf, &end);
        if(errno == ERANGE){
            isErrorF = 1;
            errno = 0;
        }
        if(end == buf){
            isErrorF = 1;
        }
    }

    void fromNumber(float num){
        dtostrf(num, 0, FLOAT_DIGITS_AFTER_DECIMAL, buf);
    }

    bool isBufferOverflow(){
        auto temp = isBufferOverflowF;
        isBufferOverflowF = 0;
        return temp;
    }

    bool isError(){
        auto temp = isErrorF;
        isErrorF = 0;
        return temp;
    }
};

#endif
