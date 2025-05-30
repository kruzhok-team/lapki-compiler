#ifndef UNISTRING_HPP
#define UNISTRING_HPP
#include <charconv>
#include <cstdlib>
#include <string>
#include <type_traits>
// TODO проверять на размеры чисел, чтобы избежать переполнения
// проверять результат std::to_chars

constexpr int bufSize = 64;

template <typename T>
auto check_operator_string_impl(const T& t)
    -> decltype(static_cast<std::string>(t), std::true_type{});
std::false_type check_operator_string_impl(...);

template <typename T>
constexpr bool has_operator_string =
    decltype(check_operator_string_impl(std::declval<T>()))::value;

// works with primitive types and everything that has operator std::string
struct UniString {
    std::string value;

    operator std::string() const { return value; }

    template <typename T>
    static constexpr bool is_supported_integer =
        std::is_integral_v<T> && (std::is_same_v<T, long long> ||
                                  std::is_same_v<T, unsigned long long> ||
                                  sizeof(T) <= sizeof(long long));

    template <typename T>
    bool try_parse(T& out) const {
        char* end_ptr = nullptr;
        const char* str = value.c_str();
        errno = 0;

        if constexpr (std::is_same_v<T, long long>) {
            long long tmp = strtoll(str, &end_ptr, 10);
            if (errno == 0 && end_ptr == str + value.size()) {
                out = tmp;
                return true;
            }
        } else if constexpr (std::is_same_v<T, unsigned long long>) {
            unsigned long long tmp = strtoull(str, &end_ptr, 10);
            if (errno == 0 && end_ptr == str + value.size()) {
                out = tmp;
                return true;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            double tmp = strtod(str, &end_ptr);
            if (errno == 0 && end_ptr == str + value.size()) {
                out = static_cast<T>(tmp);
                return true;
            }
        }
        return false;
    }

    template <typename T, typename = std::enable_if_t<
                              std::is_arithmetic_v<T> ||
                              std::is_convertible_v<T, std::string>>>
    explicit UniString(T num) {
        if constexpr (std::is_same_v<T, bool>) {
            value = num ? "true" : "false";
        } else if constexpr (std::is_integral_v<T>) {
            char buffer[bufSize];

            if constexpr (std::is_unsigned_v<T>) {
                std::to_chars_result res =
                    std::to_chars(buffer, buffer + sizeof(buffer),
                                  static_cast<unsigned long long>(num));
                value.assign(buffer, res.ptr);
            } else {
                std::to_chars_result res =
                    std::to_chars(buffer, buffer + sizeof(buffer),
                                  static_cast<long long>(num));
                value.assign(buffer, res.ptr);
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            char buffer[bufSize];
            auto [res, ec] =
                std::to_chars(buffer, buffer + sizeof(buffer), num);
            value.assign(buffer, res);
        } else if constexpr (has_operator_string<T>) {
            value = static_cast<std::string>(num);
        } else {
            static_assert(
                sizeof(T) == 0,
                "Incompatible data type is provided for UniString\nAt least be "
                "castable to std::string");
        }
    }

    explicit UniString(const bool v) : value(v ? "true" : "false") {}
    explicit UniString(const char* s) : value(s) {}
    UniString() = default;
    UniString(const std::string s) : value(s) {}

    template <typename T>
    bool operator==(const T& other) const {
        if constexpr (is_supported_integer<T>) {
            T parsed;
            return try_parse(parsed) && parsed == other;
        } else if constexpr (std::is_floating_point_v<T>) {
            double parsed;
            return try_parse(parsed) && parsed == other;
        } else {
            return value == static_cast<std::string>(other);
        }
    }

    operator int() const {
        double self_num;
        bool self_is_num = try_parse(self_num);
        if (self_is_num) {
            return (int) self_num;
        }
        return 0;
    }

    template <typename T>
    bool operator<(const T& other) const {
        if constexpr (is_supported_integer<T> || std::is_floating_point_v<T>) {
            double self_num, other_num = static_cast<double>(other);
            bool self_is_num = try_parse(self_num);

            if (self_is_num)
                return self_num < other_num;
            else
                static_assert(sizeof(T) == 0,
                              "No comparison method for UniString and number: "
                              "UniString is "
                              "nan");
        } else {
            return value < static_cast<std::string>(other);
        }
    }

    UniString& operator=(const UniString& other) = default;

    template <typename T>
    inline UniString& operator=(const T& other) {
        *this = UniString(other);
        return *this;
    }

    template <typename T>
    inline bool operator!=(const T& other) const {
        return !(*this == other);
    }

    template <typename T>
    inline bool operator>(const T& other) const {
        return other < *this;
    }

    template <typename T>
    inline bool operator<=(const T& other) const {
        return !(other < *this);
    }

    template <typename T>
    inline bool operator>=(const T& other) const {
        return !(*this < other);
    }
};

template <typename T>
bool operator==(const T& lhs, const UniString& rhs) {
    return rhs == lhs;
}

template <typename T>
bool operator!=(const T& lhs, const UniString& rhs) {
    return rhs != lhs;
}

template <typename T>
bool operator<(const T& lhs, const UniString& rhs) {
    return rhs > lhs;
}

template <typename T>
bool operator>(const T& lhs, const UniString& rhs) {
    return rhs < lhs;
}

template <typename T>
bool operator<=(const T& lhs, const UniString& rhs) {
    return rhs >= lhs;
}

template <typename T>
bool operator>=(const T& lhs, const UniString& rhs) {
    return rhs <= lhs;
}

#endif