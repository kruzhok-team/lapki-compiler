#include <stm32g030xx.h>  // CMSIS
#include "system.c"
#define INLINE__ __attribute__((always_inline))

using byte = uint8_t;

#include "sketch.cpp"

int main() {

    while (1) {

        setup();
        while (1) { loop(); }
    }

    return 0;
}