/*
 * kl_string.cpp
 *
 *  Created on: 5 дек. 2020 г.
 *      Author: Kreyl
 */

#include "kl_string.h"

__inline__ __attribute__((__always_inline__))
int kl_tolower(char c) {
    return (c >= 'A' and c <= 'Z')? (c + ('a' - 'A')) : c;
}

/* Compare S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int kl_strcasecmp(const char *s1, const char *s2) {
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  int result;
  if (p1 == p2) return 0;
  while((result = kl_tolower(*p1) - kl_tolower(*p2++)) == 0) {
      if(*p1++ == '\0') break;
  }
  return result;
}

char* kl_strtok(char* s, const char* delim, char**PLast) {
    if(s == nullptr and (s = *PLast) == nullptr) return nullptr;
    char* spanp;
    // Skip leading delimiters
    cont:
    char c = *s++, sc;
    for(spanp = (char*)delim; (sc = *spanp++) != 0;) {
        if(c == sc) goto cont;
    }

    if(c == 0) {    // no non-delimiter characters left, but string ended
        *PLast = nullptr;
        return nullptr;
    }
    char* tok = s - 1;
    while(true) {
        c = *s++;
        spanp = (char*)delim;
        do {
            if((sc = *spanp++) == c) {
                if(c == 0) s = nullptr;
                else *(s-1) = 0;
                *PLast = s;
                return tok;
            }
        } while (sc != 0);
    }
}

int kl_strlen(const char* s) {
    const char *fs;
    for(fs = s; *fs; ++fs);
    return (fs - s);
}

#ifndef ULONG_MAX
#define ULONG_MAX   ((unsigned long)(~0L))      /* 0xFFFFFFFF */
#endif

#ifndef LONG_MAX
#define LONG_MAX    ((long)(ULONG_MAX >> 1))    /* 0x7FFFFFFF */
#endif

#ifndef LONG_MIN
#define LONG_MIN    ((long)(~LONG_MAX))     /* 0x80000000 */
#endif

// tab; newline; vertical-tab; form-feed; carriage-return; space;
static inline bool kl_isspace(char c) { return (c >= 0x09 and c <=0x0D) or (c == ' '); }

// 1; 2; 3; 4; 5; 6; 7; 8; 9; 0;
static inline bool kl_isdigit(char c) { return (c >= '0' and c <= '9'); }

// A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W; X; Y; Z; a; b; c; d; e; f; g; h; i; j; k; l; m; n; o; p; q; r; s; t; u; v; w; x; y; z;
static inline bool kl_isalpha(char c) { return (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z'); }

// A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W; X; Y; Z;
static inline bool kl_isupper(char c) { return (c >= 'A' and c <= 'Z'); }

long kl_strtol(const char* nptr, char** endptr, int base) {
    const char *s = nptr;
    unsigned long acc;
    int c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while(kl_isspace(c));
    if(c == '-') {
        neg = 1;
        c = *s++;
    }
    else if(c == '+') c = *s++;
    if((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if(base == 0) base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(unsigned long) LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long) base;
    cutoff /= (unsigned long) base;
    for(acc = 0, any = 0;; c = *s++) {
        if(kl_isdigit(c)) c -= '0';
        else if(kl_isalpha(c)) c -= kl_isupper(c) ? 'A' - 10 : 'a' - 10;
        else break;
        if(c >= base) break;
        if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if(any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
    }
    else if(neg) acc = -acc;
    if(endptr != 0) *endptr = (char*) (any ? s - 1 : nptr);
    return (acc);
}

