#pragma once

/*
    UART1 implementation for user code (bootloader)
    tested on main-a4, btn-a2
*/

#include "Pins.hpp"
#include "macros.hpp"

namespace detail {

    namespace debug {

        bool debugLedIndicator = false;
        uint32_t timer = 0;
    }

    namespace constants {

        // ReDe подцеплен к PA0
        GPIO_TypeDef* const ReDePort = GPIOA;
        const uint8_t ReDeNum = 0;

        const uint8_t RECEIVE_MODE = 1;
        const uint8_t SEND_MODE = 2;

        const uint8_t CR = 13;   // value for begin msg packet
        const uint8_t LF = 10;   // value for end msg packet
    }

    namespace helper {

        // Инициализация пина ReDe
        INLINE__ STATIC__ void initReDe(void) {

            initPin_OD(detail::constants::ReDePort, detail::constants::ReDeNum);
        }

        // Управление драйвером RS485 (MAX3441)
        // Для аргумента использовать константы выше из detail::constants (RECEIVE_MODE, SEND_MODE)
        INLINE__ STATIC__
        void setReDe(const uint8_t x) {
            
            setPin_OD(detail::constants::ReDePort, detail::constants::ReDeNum, x);
        }
    }

    namespace simpleBusHelpers {

        uint8_t addr;
        uint16_t addrAsci;  // for quick check in interrupt function
        bool isByteReceived;
        uint8_t receivedByte;

        uint8_t buffer[5];
        uint8_t packetIterator = 0;
    }

    namespace dataBusHelpers {

        bool isByteReceived;
        uint8_t receivedByte;
    }

    namespace hal {

        // Настройка USART2
        // t127 p826 RM0454 // Таблица прерываний UART
        INLINE__ STATIC__ 
        void initUART(const uint32_t baudrate) {
            
            // 1. Включить тактование
            // Включаем модуль UART (Включаем тактирование USART1 от шины APB)
            RCC -> APBENR2 |= RCC_APBENR2_USART1EN;

            // 2. Настроить ноги
            // Включаем тактирование ног UART (Включаем тактирование модуля GPIOA)
            RCC -> IOPENR |= RCC_IOPENR_GPIOAEN;  // also for ReDe!
            RCC -> IOPENR |= RCC_IOPENR_GPIOBEN;

            // Настраиваем ноги контроллера для работы с UART
            // UART1 на ногах PB6 {AF=1, Tx}, PB7 {AF=1, Rx}
            initPin_AF_OD ( GPIOB, 6, 0 ); //[A.1][B.2]
            initPin_AF_OD ( GPIOB, 7, 0 ); //[A.2][B.2]

            // 3. Настроить скорость
            // Установка скорости UART
            USART1 -> BRR |= (uint32_t)( PCLK/baudrate ); // { Baudrate = PCLK / BRR } -> { BRR = PCLK / Baudrate }

            // 4. Включить прерывание по Rx
            // Включаем прерывание по Rx - RXNE (Rx buffer not empty), оно склеено с прерыванием RXFNE для режима FIFO
            // s26.8.2 p831 RM0454 // Описание регистра CR1
            USART1 -> CR1 |= USART_CR1_RXNEIE_RXFNEIE;

            // 5. Включить сам модуль
            // Включаем работу модуля
            USART1 -> CR1 |= USART_CR1_UE;

            // 6. Включить приёмник
            // Включаем приёмник
            USART1 -> CR1 |= USART_CR1_RE;

            // 7. Включить передатчик
            // Включаем передатчик
            USART1 -> CR1 |= USART_CR1_TE;

            // 8. Активировать прерывание в NVIC
            // Разрешаем прерывание UART1: UART1_IRQn
            // s11 p250 RM0454 // Описание NVIC
            NVIC_EnableIRQ(USART1_IRQn);

            // Инициализируем и переводим в режим приёма пин ReDe
            detail::helper::initReDe();
            detail::helper::setReDe(detail::constants::RECEIVE_MODE);
        }

        // Обёртка над setReDe, уменьшающая путаницу (ждём, чтобы модуль UART завершил передачу, затем включаем приём)
        // p850 RM0454 - Описание флага TC
        void setReDeReceive(void) {

            while (((USART1 -> ISR) & USART_ISR_TC ) == 0);
            detail::helper::setReDe(detail::constants::RECEIVE_MODE);
        }

        // Обёртка над setReDe, уменьшающая путаницу (включаем передачу)
        void setReDeTransmit(void) {

            detail::helper::setReDe(detail::constants::SEND_MODE);
        }
        
        // Передача одного символа (семантика ReDe вовне этой функции)
        void putChar(const uint8_t x) {

            // Ждем, пока значение регистра TDR будет обработано
            while ((USART1 -> ISR & USART_ISR_TXE_TXFNF) == 0);

            // Отправляем байт
            USART1 -> TDR = x;

            // for gebug
            detail::debug::debugLedIndicator = true;
        }

        namespace api {
            
            // Ниже функции для прерывания, нужно выбрать одну подходящую.
            // Но их линкер не найдет. Поэтому требуется функция обертка, которую он найдет
            // Эта функция-обертка находится непосредственно, либо в DataBus.hpp, либо в SimpleBus.hpp
            
            // Функция для DataBus
            void USART1_IRQHandlerDataBus(void) {

                using namespace detail::dataBusHelpers;

                // Читаем пришедшее значение (других источников у этого прерывания нет)
                uint8_t gotByte = USART1 -> RDR;

                // Сохраняем пришедшее значение
                receivedByte = gotByte;
                isByteReceived = true;

                // TODO: Не нужно, этот флаг сбрасывается сразу при чтении из RDR
                USART1 -> RQR |= USART_RQR_RXFRQ;

                // debug
                detail::debug::debugLedIndicator = true;
            }
            
            // Функция для SimpleBus
            void USART1_IRQHandlerSimpleBus(void) {

                using namespace detail::simpleBusHelpers;
                using namespace detail::constants;

                // Читаем пришедшее значение (других источников у этого прерывания нет)
                uint8_t gotByte = USART1 -> RDR;

                // save to buffer
                buffer[packetIterator++] = gotByte;

                // if packet ready (got 5 bytes in buffer) then parse
                if (packetIterator >= 5) {

                    if (buffer[0] == CR && buffer[4] == LF) {   // packet is correct

                        // pull addr (При сдвигах начинается лютая жесть, поэтому в целях совместимости с чипом лучше оставить так)
                        // uint16_t targetAddr = ((*reinterpret_cast<uint16_t*>(&buffer[1])) << 8) | buffer[2];
                        uint16_t targetAddr = buffer[1];
                        targetAddr = (targetAddr << 8) | buffer[2];
                        
                        if (targetAddr == addrAsci) {

                            receivedByte = buffer[3];
                            isByteReceived = true;
                        }
                        packetIterator = 0; // reset buffer
                    }
                    else {  // packet not correct -> shift to one byte?

                        *reinterpret_cast<uint16_t* const>(&buffer[0]) = *reinterpret_cast<uint16_t const * const>(&buffer[1]);
                        *reinterpret_cast<uint16_t* const>(&buffer[2]) = *reinterpret_cast<uint16_t const * const>(&buffer[3]);
                        --packetIterator; // decrease buffer
                    }
                }

                // TODO: Не нужно, этот флаг сбрасывается сразу при чтении из RDR
                USART1 -> RQR |= USART_RQR_RXFRQ;

                // debug
                detail::debug::debugLedIndicator = true;
            }
        }
    }
}