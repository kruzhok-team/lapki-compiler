#ifndef SERIAL_HPP
#define SERIAL_HPP
#include <string>

#include "UniString.h"

// было сделано так, потому что требуется, чтобы переменная была доступна в
// лапках без промежуточных вызовов для преобразования типов

class Serial {
    static inline UniString message_ = UniString("");
    static inline bool isUpdated_ = 0;

   public:
    static inline const UniString& message = message_;
    Serial() = delete;

    static void setMessage(char* str);

    static void setMessage(std::string str);

    static inline bool isUpdated() {
        bool t = isUpdated_;
        isUpdated_ = 0;
        return t;
    }

    static void Printf(const char* format, ...);

    // в shell.cpp отключен вывод float
    // l (например %lld) не обрабатывается в shell.cpp
    // от того возинкает переполнение

    template <typename T>
    static void printVal(const char* txt, const T& val) {
        UniString temp;
        if constexpr (std::is_convertible_v<T, std::string>)
            temp = UniString(static_cast<std::string>(val));
        else if constexpr (std::is_floating_point_v<T>)
            temp = UniString(static_cast<long long>(val));
        else
            temp = UniString(val);
        Printf("%s %s\n\r", txt, temp.value.c_str());
    }

    template <typename T>
    static void print(const T& msg) {
        UniString temp;
        if constexpr (std::is_convertible_v<T, std::string>)
            temp = UniString(static_cast<std::string>(msg));
        else if constexpr (std::is_floating_point_v<T>)
            temp = UniString(static_cast<long long>(msg));
        else
            temp = UniString(msg);
        Printf("%s\n\r", temp.value.c_str());
    };
};

#endif  // SERIAL_HPP