/*
 * shell.h
 *
 *  Created on: 25 ���. 2015 �.
 *      Author: Kreyl
 */

#ifndef SHELL_H_
#define SHELL_H_

#include <cstring>
#include <stdarg.h>
#include "gd_lib.h"
#include "board.h"
#include "color.h"
#include "kl_string.h"
#include "yartos.h"

#define DELIMITERS              " ,"
#define PREV_CHAR_TIMEOUT_ms    99UL

enum ProcessDataResult_t {pdrProceed, pdrNewCmd};

class Cmd {
private:
    char istring[CMD_BUF_SZ];
    char* remainer = nullptr;
    uint32_t cnt;
    bool completed;
    systime_t last_char_timestamp = 0;
    bool IsSpace(char c) { return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' '); }
    bool IsDigit(char c) { return c >= '0' and c <= '9'; }
public:
    char *name;
    ProcessDataResult_t PutChar(char c) {
        // Reset cmd: (1) if it was completed and after that new char arrived (2) if new char has come after long pause
        if(completed or Sys::TimeElapsedSince(last_char_timestamp) > TIME_MS2I(PREV_CHAR_TIMEOUT_ms)) {
            completed = false;
            cnt = 0;
        }
        last_char_timestamp = Sys::GetSysTimeX();
        // Process char
        if(c == '\b') { if(cnt > 0) cnt--; }    // do backspace
        else if((c == '\r') or (c == '\n')) {   // end of line, check if cmd completed
            if(cnt != 0) {  // if cmd is not empty
                istring[cnt] = 0; // End of string
                name = kl_strtok(istring, DELIMITERS, &remainer);
                completed = true;
                return pdrNewCmd;
            }
        }
        else if(cnt < (CMD_BUF_SZ-1)) istring[cnt++] = c;  // Add char if buffer not full
        return pdrProceed;
    }

    char* GetNextString() { return kl_strtok(nullptr, DELIMITERS, &remainer); }

    char* GetRemainder() { return remainer; }

    template <typename T>
    retv GetNext(T *poutput) {
        char* S = GetNextString();
        if(S) {
            char *p;
            int32_t dw32 = kl_strtol(S, &p, 0);
            if(*p == '\0') {
                *poutput = (T)dw32;
                return retv::Ok;
            }
            else return retv::NotANumber;
        }
        return retv::Fail;
    }

    retv GetNextU8(uint8_t *poutput) {
        char* S = GetNextString();
        if(S) {
            char *p;
            uint8_t v = strtoul(S, &p, 0);
            if(*p == '\0') { *poutput = v; return retv::Ok; }
            else return retv::NotANumber;
        }
        return retv::Fail;
    }

    retv GetNextU16(uint16_t *poutput) {
        char* S = GetNextString();
        if(S) {
            char *p;
            uint16_t v = strtoul(S, &p, 0);
            if(*p == '\0') { *poutput = v; return retv::Ok; }
            else return retv::NotANumber;
        }
        return retv::Fail;
    }

    retv GetNextI16(int16_t *poutput) {
        char* S = GetNextString();
        if(S) {
            char *p;
            int16_t v = kl_strtol(S, &p, 0);
            if(*p == '\0') { *poutput = v; return retv::Ok; }
            else return retv::NotANumber;
        }
        return retv::Fail;
    }

    retv GetNextI32(int32_t *poutput) {
        char* S = GetNextString();
        if(S) {
            char *p;
            int32_t v = kl_strtol(S, &p, 0);
            if(*p == '\0') { *poutput = v; return retv::Ok; }
            else return retv::NotANumber;
        }
        return retv::Fail;
    }

    /*
     * Codes:
     * ints: %u32 %u16 %u8 %d32 %d16 %d8
     * string: %S %s
     * float: %f
     * '*' means skip next: %*
     * Returns number of successful conversions
     */
    int Get(const char* fmt, ...) {
        int N = 0;
        va_list args;
        va_start(args, fmt);
        while(true) {
            char c = *fmt++;
            if(c == 0) goto End;
            if(c != '%') continue;
            // % found, what next?
            c = *fmt++;
            // Check if skip next token
            if(c == '*') {
                if(GetNextString() == nullptr) goto End;
                else continue;
            }

            // Get next token
            char *tok = kl_strtok(nullptr, DELIMITERS, &remainer);
            if(tok == nullptr) goto End;

            // Command decoding
            switch(c) {
                case 'u': {
                    // Convert
                    char *ret;
                    uint32_t v = strtoul(tok, &ret, 0); // Conversion failed
                    if(*ret != 0) goto End;
                    // Get sz
                    c = *fmt++;
                    if(c == '8') {
                        uint8_t *p = va_arg(args, uint8_t*);
                        if(p == nullptr) goto End;
                        *p = (uint8_t)v;
                    }
                    else if(c == '1') {
                        uint16_t *p = va_arg(args, uint16_t*);
                        if(p == nullptr) goto End;
                        *p = (uint16_t)v;
                    }
                    else if(c == '3') {
                        uint32_t *p = va_arg(args, uint32_t*);
                        if(p == nullptr) goto End;
                        *p = v;
                    }
                    else goto End;
                    N++;
                }
                break;

                case 'd': {
                    // Convert
                    char *ret;
                    int32_t v = kl_strtol(tok, &ret, 0); // Conversion failed
                    if(*ret != 0) goto End;
                    // Get sz
                    c = *fmt++;
                    if(c == '8') {
                        int8_t *p = va_arg(args, int8_t*);
                        if(p == nullptr) goto End;
                        *p = (int8_t)v;
                    }
                    else if(c == '1') {
                        int16_t *p = va_arg(args, int16_t*);
                        if(p == nullptr) goto End;
                        *p = (int16_t)v;
                    }
                    else if(c == '3') {
                        int32_t *p = va_arg(args, int32_t*);
                        if(p == nullptr) goto End;
                        *p = v;
                    }
                    else goto End;
                    N++;
                }
                break;

                case 's':
                case 'S': {
                    char **p = va_arg(args, char**);
                    if(p == nullptr) goto End;
                    *p = tok;
                    N++;
                }
                break;
#if PRINTF_FLOAT_EN
                case 'f': {
                    float *p = va_arg(args, float*);
                    if(p == nullptr) goto End;
                    char *ret;
                    *p = strtof(tok, &ret);
                    if(*ret != 0) goto End; // Conversion failed
                    N++;
                }
                break;
#endif
                default: break; // including '*'
            } // switch c
        } // while true
        End:
        return N;
    }

#if PRINTF_FLOAT_EN
    uint8_t GetNextFloat(float *poutput) {
        char* S = GetNextString();
        if(!S) return retv::Fail;
        char *p;
        float f = strtof(S, &p);
        if(*p == '\0') {
            *poutput = f;
            return retv::Ok;
        }
        else return retvNotANumber;
    }

    uint8_t GetNextDouble(double *poutput) {
        char* S = GetNextString();
        if(!S) return retv::Fail;
        char *p;
        double f = strtod(S, &p);
        if(*p == '\0') {
            *poutput = f;
            return retv::Ok;
        }
        else return retvNotANumber;
    }
#endif

    template <typename T>
    retv GetArray(T *ptr, int32_t len) {
        for(int32_t i=0; i<len; i++) {
            T number;
            retv r = GetNext<T>(&number);
            if(r == retv::Ok) *ptr++ = number;
            else return r;
        }
        return retv::Ok;
    }

    retv GetClrRGB(Color_t *pclr) {
        if(GetNext<uint8_t>(&pclr->R) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->G) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->B) != retv::Ok) return retv::Fail;
        return retv::Ok;
    }

    retv GetClrRGBW(Color_t *pclr) {
        if(GetNext<uint8_t>(&pclr->R) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->G) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->B) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->W) != retv::Ok) return retv::Fail;
        return retv::Ok;
    }

    retv GetClrHSV(ColorHSV_t *pclr) {
        if(GetNext<uint16_t>(&pclr->H) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->S) != retv::Ok) return retv::Fail;
        if(GetNext<uint8_t>(&pclr->V) != retv::Ok) return retv::Fail;
        return retv::Ok;
    }

    /*  int32_t Indx, value;
        if(PCmd->GetParams<int32_t>(2, &Indx, &value) == retv::Ok) {...}
        else PShell->Ack(retvCmdError);    */
    template <typename T>
    retv GetParams(uint8_t cnt, ...) {
        retv Rslt = retv::Ok;
        va_list args;
        va_start(args, cnt);
        while(cnt--) {
            T* ptr = va_arg(args, T*);
            Rslt = GetNext<T>(ptr);
            if(Rslt != retv::Ok) break;
        }
        va_end(args);
        return Rslt;
    }

    bool NameIs(const char *S) { return (kl_strcasecmp(name, S) == 0); }
    Cmd() {
        cnt = 0;
        completed = false;
        name = nullptr;
    }
};

// Parent class for everything that prints
class PrintfHelper {
private:
    retv IPutUint(uint32_t n, uint32_t base, uint32_t width, char filler);
protected:
    virtual retv IPutChar(char c) = 0;
    virtual void IStartTransmissionIfNotYet() = 0;
public:
    void IVsPrintf(const char *format, va_list args);
    void Print(const char *format, ...) {
        va_list args;
        va_start(args, format);
        IVsPrintf(format, args);
        va_end(args);
    }
    void PrintEOL() {
        IPutChar('\r');
        IPutChar('\n');
        IStartTransmissionIfNotYet();
    }
};

class Shell : public PrintfHelper {
public:
	Cmd cmd;
    void Ok()  { Print("Ok\r\n"); }
    void BadParam() { Print("BadParam\r\n"); }
    void CRCError() { Print("CRCError\r\n"); }
    void CmdError() { Print("CmdError\r\n"); }
    void CmdUnknown() { Print("CmdUnknown\r\n"); }
    void Failure()  { Print("Failure\r\n");  }
    void Timeout()  { Print("Timeout\r\n");  }
    void NoAnswer() { Print("NoAnswer\r\n"); }
    void Overflow() { Print("Overflow\r\n"); }
	virtual retv ReceiveBinaryToBuf(uint8_t *ptr, uint32_t len, uint32_t timeout_ms) = 0;
	virtual retv TransmitBinaryFromBuf(uint8_t *ptr, uint32_t len, uint32_t timeout_ms) = 0;
};

// Functions
class CmdUart;

void Printf(const char *format, ...);
void Printf(CmdUart &auart, const char *format, ...);
void PrintfI(const char *format, ...);
void PrintfEOL();
void PrintfNow(const char *S);

extern "C" {
void PrintfC(const char *format, ...);
//void PrintfCNow(const char *format, ...);
}

#endif // SHELL_H_
