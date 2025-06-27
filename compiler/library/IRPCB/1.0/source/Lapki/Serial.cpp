#include "Serial.h"

#include "usb_msdcdc.h"
void Serial::setMessage(char* str) {
    message_ = str;
    isUpdated_ = 1;
}

void Serial::setMessage(std::string str) {
    message_ = str;
    isUpdated_ = 1;
}

void Serial::Printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    usb_msd_cdc.IVsPrintf(format, args);
    va_end(args);
}