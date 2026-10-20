#pragma once
#include "AtmRuntime.hpp"

namespace atm { namespace uart {
// A single SPSC queue is shared by the mutually exclusive bus components.
static volatile uint8_t queue[64], head = 0, tail = 0;
inline void begin(uint32_t baudrate) {
    if (baudrate < 300 || baudrate > 1000000) baudrate = 9600;
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    pinMode(GPIOA, 0, 1); // PA0 high: receive; U3 inverts the request.
    pinMode(GPIOB, 6, 2, 0);
    pinMode(GPIOB, 7, 2, 0);
    USART1->CR1 = 0;
    USART1->BRR = (16000000u + baudrate / 2) / baudrate;
    USART1->ICR = 0xffffffff;
    head = tail = 0;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE;
    NVIC_SetPriority(USART1_IRQn, 2);
    NVIC_EnableIRQ(USART1_IRQn);
}
inline bool pop(uint8_t& value) {
    const uint8_t current = tail;
    if (current == head) return false;
    value = queue[current];
    tail = (current + 1) & 63;
    return true;
}
inline bool wait(uint32_t flag, uint32_t start) {
    while (!(USART1->ISR & flag))
        if (uint32_t(millis() - start) >= 100) return false;
    return true;
}
inline void send(const uint8_t* bytes, uint8_t count) {
    const uint32_t start = millis();
    GPIOA->BSRR = 1u << 16; // request transmit; M1 must also permit the bus.
    USART1->ICR = USART_ICR_TCCF;
    for (uint8_t i = 0; i < count; ++i) {
        if (!wait(USART_ISR_TXE_TXFNF, start)) break;
        USART1->TDR = bytes[i];
    }
    wait(USART_ISR_TC, start);
    GPIOA->BSRR = 1; // release even on timeout
}
}}

extern "C" void USART1_IRQHandler() {
    const uint32_t flags = USART1->ISR;
    if (flags & USART_ISR_RXNE_RXFNE) {
        const uint8_t value = USART1->RDR;
        const uint8_t next = (atm::uart::head + 1) & 63;
        // Full queue drops the new byte, preserving unread data.
        if (next != atm::uart::tail) {
            atm::uart::queue[atm::uart::head] = value;
            atm::uart::head = next;
        }
    }
    USART1->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_PECF;
}
