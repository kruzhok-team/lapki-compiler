#include "gd_uart.h"
#include "shell.h"
#include "yartos.h"

extern CmdUart dbg_uart;

void Printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Sys::Lock();
    dbg_uart.IVsPrintf(format, args);
    Sys::Unlock();
    va_end(args);
}

void Printf(CmdUart &auart, const char *format, ...) {
    va_list args;
    va_start(args, format);
    Sys::Lock();
    auart.IVsPrintf(format, args);
    Sys::Unlock();
    va_end(args);
}

void PrintfI(const char *format, ...) {
    va_list args;
    va_start(args, format);
    dbg_uart.IVsPrintf(format, args);
    va_end(args);
}

void PrintfNow(const char *S) {
    dbg_uart.StopTx();
    dbg_uart.PrintfNow(S);
}

void PrintfEOL() {
    dbg_uart.PrintEOL();
}

extern "C" {
void PrintfC(const char *format, ...) {
    va_list args;
    va_start(args, format);
    dbg_uart.IVsPrintf(format, args);
    va_end(args);
}
} // extern C


class PrintToBuf : public PrintfHelper {
public:
    char *S;
    retv IPutChar(char c) {
        *S++ = c;
        return retv::Ok;
    }
    void IStartTransmissionIfNotYet() {}
};

char* PrintfToBuf(char* pbuf, const char *format, ...) {
    PrintToBuf print_to_buf;
    print_to_buf.S = pbuf;
    va_list args;
    va_start(args, format);
    print_to_buf.IVsPrintf(format, args);
    va_end(args);
    *print_to_buf.S = 0;
    return print_to_buf.S;
}

#if PRINTF_FLOAT_EN
#define FLOAT_PRECISION     9
static const long power10Table[FLOAT_PRECISION] = {
    10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};
#endif

void PrintfHelper::IVsPrintf(const char *format, va_list args) {
    const char *fmt = format;
    int width = 0, precision;
    char c, filler;
    while(true) {
        c = *fmt++;
        if(c == 0) goto End;
        if(c != '%') {  // Not %
            if(IPutChar(c) != retv::Ok) goto End;
            else continue;
        }

        // Here goes optional width specification.
        // If it starts with zero (zero_padded is true), it means we use '0' instead of ' ' as a filler.
        filler = ' ';
        if(*fmt == '0') {
            fmt++;
            filler = '0';
        }

        width = 0;
        while(true) {
            c = *fmt++;
            if(c >= '0' && c <= '9') c -= '0';
            else if (c == '*') c = va_arg(args, int);
            else break;
            width = width * 10 + c;
        }

        precision = 0;
        if(c == '.') {
            while(true) {
                c = *fmt++;
                if(c >= '0' && c <= '9') c -= '0';
                else if(c == '*') c = va_arg(args, int);
                else break;
                precision = precision * 10 + c;
            }
        }

        // Command decoding
        switch(c) {
            case 'c':
                if(IPutChar(va_arg(args, int)) != retv::Ok) goto End;
                break;

            case 's':
            case 'S': {
                char *s = va_arg(args, char*);
                width -= kl_strlen(s); // Do padding of string
                while(s and *s)    { if(IPutChar(*s++)   != retv::Ok) goto End; }
                while(width-- > 0) { if(IPutChar(filler) != retv::Ok) goto End; } // Do padding of string
            }
            break;

            case 'X':
                if(IPutUint(va_arg(args, uint32_t), 16, width, filler) != retv::Ok) goto End;
                break;
            case 'u':
                if(IPutUint(va_arg(args, uint32_t), 10, width, filler) != retv::Ok) goto End;
                break;

            case 'd':
            case 'i':
            {
                int32_t n = va_arg(args, int32_t);
                if(n < 0) {
                    if(IPutChar('-') != retv::Ok) goto End;
                    n = -n;
                }
                if(IPutUint(n, 10, width, filler) != retv::Ok) goto End;
            }
            break;

#if PRINTF_FLOAT_EN
            case 'f': {
                float f = (float)va_arg(args, double);
                if (f < 0) {
                    if(IPutChar('-') != retv::Ok) goto End;
                    f = -f;
                }
                int32_t n;
                if((precision == 0) || (precision > FLOAT_PRECISION)) precision = FLOAT_PRECISION;
                n = (int32_t)f;
                if(IPutUint(n, 10, width, filler) != retv::Ok) goto End;
                if(IPutChar('.') != retv::Ok) goto End;
                filler = '0';
                width = precision;
                n = (long)((f - n) * power10Table[precision - 1]);
                if(IPutUint(n, 10, width, filler) != retv::Ok) goto End;
            } break;
#endif

            case 'A': {
                uint8_t *arr = va_arg(args, uint8_t*);
                int32_t n = va_arg(args, int32_t);
                int32_t Delimiter = va_arg(args, int32_t);
                filler = '0';       // }
                width = 2;          // } 01 02 0A etc.; not 1 2 A
                for(int32_t i = 0; i < n; i++) {
                    if((i > 0) && (Delimiter != 0)) { // do not place delimiter before or after array
                        if(IPutChar((char)Delimiter) != retv::Ok) goto End;
                    }
                    if(IPutUint(arr[i], 16, width, filler) != retv::Ok) goto End;
                }
            } break;

            case '%':
                if(IPutChar('%') != retv::Ok) goto End;
                break;
        } // switch
    } // while
    End:
    IStartTransmissionIfNotYet();
}

retv PrintfHelper::IPutUint(uint32_t n, uint32_t base, uint32_t width, char filler) {
    char digits[10];
    uint32_t len = 0;
    // Place digits to buffer
    do {
        uint32_t digit = n % base;
        n /= base;
        digits[len++] = (digit < 10)? '0'+digit : 'A'+digit-10;
    } while(n > 0);
    // Add padding
    for(uint32_t i = len; i < width; i++) {
        if(IPutChar(filler) != retv::Ok) return retv::Overflow;
    }
    // Print digits
    while(len > 0) {
        if(IPutChar(digits[--len]) != retv::Ok) return retv::Overflow;
    }
    return retv::Ok;
} // IPutUint
