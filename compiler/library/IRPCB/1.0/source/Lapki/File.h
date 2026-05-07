#ifndef FILE_H
#define FILE_H

#include <string>

#include "kl_fs_utils.h"

//XXX не работает

class File {
   public:
    const std::string name;
    File(const std::string name) : name(name) {}

    template <typename T>
    void printVal(const char* txt, const T& val) {
        printVal(this->name, txt, val);
    }

    template <typename T>
    void print(const T& msg) {
        print(this->name, msg);
    }

    // Это в лапки не идёт
    
    //Sys::Lock()
    static void Printf(const std::string& name, const char* format, ...) {
        // Sys::Lock();
        if (f_open(&common_file, name.c_str(),
                   FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
            Sys::Unlock();
            return;
        }

        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        f_puts(buffer, &common_file);
        f_close(&common_file);

        // Sys::Unlock();
    }

    template <typename T>
    static void printVal(const std::string& name, const char* txt,
                               const T& val) {
        UniString temp;
        if constexpr (std::is_convertible_v<T, std::string>)
            temp = UniString(static_cast<std::string>(val));
        else if constexpr (std::is_floating_point_v<T>)
            temp = UniString(static_cast<long long>(val));
        else
            temp = UniString(val);
        Printf(name, "%s %s\r\n", txt, temp.value.c_str());
    }

    template <typename T>
    static void print(const std::string& name, const T& msg) {
        UniString temp;
        if constexpr (std::is_convertible_v<T, std::string>)
            temp = UniString(static_cast<std::string>(msg));
        else if constexpr (std::is_floating_point_v<T>)
            temp = UniString(static_cast<long long>(msg));
        else
            temp = UniString(msg);
        Printf(name, "%s\r\n", temp.value.c_str());
    }
};

#endif