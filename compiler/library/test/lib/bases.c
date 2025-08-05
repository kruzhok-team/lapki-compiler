#pragma once
#include <stdlib.h>

static inline uint8_t numToBase62Char(uint32_t number)
{
    if (number < 10)
    {
        return number + '0';
    }
    if ((number >= 10) && (number < 36))
    {
        return number - 10 + 'a';
    }
    if ((number >= 36) && (number < 62))
    {
        return number - 36 + 'A';
    }
    return '!';
}

static inline uint8_t numToBase64Char(uint32_t number)
{
    if (number < 10)
    {
        return number + '0';
    }
    if ((number >= 10) && (number < 36))
    {
        return number - 10 + 'a';
    }
    if ((number >= 36) && (number < 62))
    {
        return number - 36 + 'A';
    }
    if (number == 62)
        return '+';
    if (number == 63)
        return '/';
    return '!';
}

static inline uint8_t numToBase16Char(uint32_t number)
{
    if (number < 10)
    {
        return number + '0';
    }
    if ((number >= 10) && (number < 16))
    {
        return number - 10 + 'a';
    }
    return '!';
}

void toBase10(uint8_t *buffer, uint8_t bufsize, uint32_t value)
{
    uint8_t i;
    for (i = 1; i <= bufsize; i++)
    {
        buffer[bufsize - i] = (uint8_t)((value % 10UL) + '0');
        value /= 10;
    }
    buffer[i - 1] = '\0';
}

void toBase62(uint8_t *buffer, uint8_t bufsize, uint32_t value)
{
    uint8_t i;
    for (i = 1; i <= bufsize; i++)
    {
        buffer[bufsize - i] = numToBase62Char(value % 62);
        value /= 62;
    }
    buffer[i - 1] = '\0';
}

void toBase64(uint8_t *buffer, uint8_t bufsize, uint32_t value)
{
    uint8_t i;
    for (i = 1; i <= bufsize; i++)
    {
        buffer[bufsize - i] = numToBase64Char(value % 64);
        value /= 64;
    }
    buffer[i - 1] = '\0';
}

void toBase16(uint8_t *buffer, uint8_t bufsize, uint64_t value)
{
    uint8_t i;
    for (i = 1; i <= bufsize; i++)
    {
        buffer[bufsize - i] = numToBase16Char(value % 16);
        value /= 16;
    }
    buffer[i - 1] = '\0';
}
