#pragma once

#include "UART1.hpp"

extern "C" {

    void USART1_IRQHandler(void) {

        detail::hal::api::USART1_IRQHandlerSimpleBus();
    }
}

#ifdef UART1_BUSY
    #error "CE: UART1 used by other hpp file (it is UB for uart interrupt function)"
#else
    #define UART1_BUSY
#endif

namespace detail {

    namespace helpers {

        uint8_t byteToAsci(const uint8_t value) {

            if (value < 10) {
                return value + 48;
            }
            else {
                return value + 55;
            }
        }

        uint16_t addrToAsci(const uint8_t addr) {

            uint8_t left = (addr >> 4) & 0x0F;  // старший разряд байта
            uint8_t right = addr & 0x0F;        // младший разряд байта

            uint8_t leftA = byteToAsci(left);            // Перевели в аски
            uint8_t rightA = byteToAsci(right);          // Перевели в аски

            return (uint16_t(leftA) << 8) | rightA;
        }
    }
}

// Компонент для базового взаимодействия с общей шиной данных на основе простого протокола
// Шина является полудуплексной – отправку в конкретный момент времени может отсуществлять только один участник
// Использует UART1
/*
    Структура пакета для протокола SimpleBus:

    [CR] [addr1] [addr2] [data] [LF]

        [addr1] – старший полу-байт адреса в HEX-представлении
        [addr2] – младший полу-байт адреса в HEX-представлении
        [data] – символ полезной нагрузки
        [CR] [LF] – соответствующие управляющие символы
*/
class SimpleBus {

public:

    // ctor
    SimpleBus(const uint32_t baudrate) {

        detail::hal::initUART(baudrate);
    }

    void setAddress(const uint8_t addr) {

        detail::simpleBusHelpers::addr = addr;
        detail::simpleBusHelpers::addrAsci = detail::helpers::addrToAsci(addr);
    }

    void sendPacket(const uint8_t addr, const uint8_t data) {

        uint16_t addrAsci = detail::helpers::addrToAsci(addr);

        // Переводим драйвер в режим передачи
        detail::hal::setReDeTransmit();

        detail::hal::putChar(detail::constants::CR);

        detail::hal::putChar((addrAsci >> 8) & 0xFF);
        detail::hal::putChar(addrAsci & 0xFF);

        detail::hal::putChar(data);

        detail::hal::putChar(detail::constants::LF);
        
        // Переводим драйвер шины в режим приёма
        detail::hal::setReDeReceive();
    }

    bool packetReceived() {

        return detail::simpleBusHelpers::isByteReceived;
    }

    uint8_t lastData() {

        detail::simpleBusHelpers::isByteReceived = false;
        return detail::simpleBusHelpers::receivedByte;
    }

    uint8_t getAddress() {

        return detail::simpleBusHelpers::addr;
    }

private:
};