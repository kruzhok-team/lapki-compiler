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

// в shell.cpp отключен вывод float
// l (например %lld) не обрабатывается в shell.cpp
// от того возинкает переполнение

// template <typename T>
// void Serial::printVal(const char* txt, const T& val) {
//     UniString temp;
//     if constexpr (std::is_convertible_v<T, std::string>)
//         temp = UniString(static_cast<std::string>(val));
//     else if constexpr (std::is_floating_point_v<T>)
//         temp = UniString(static_cast<long long>(val));
//     else
//         temp = UniString(val);
//     Printf("%s %s\n\r", txt, temp.value.c_str());
// }

// template <typename T>
// void Serial::print(const T& msg) {
//     UniString temp;
//     if constexpr (std::is_convertible_v<T, std::string>)
//         temp = UniString(static_cast<std::string>(msg));
//     else if constexpr (std::is_floating_point_v<T>)
//         temp = UniString(static_cast<long long>(msg));
//     else
//         temp = UniString(msg);
//     Printf("%s\n\r", temp.value.c_str());
// }
