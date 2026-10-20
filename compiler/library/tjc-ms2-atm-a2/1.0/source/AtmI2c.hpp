#pragma once
#include "AtmRuntime.hpp"

namespace atm {
// Transactions are short and bounded. Sensor conversion waits are handled by
// the drivers' poll state machines, with no ownership of the bus between calls.
class I2cBus {
    uint8_t number;
    bool initialized = false;
    I2C_TypeDef* regs() const { return number == 1 ? I2C1 : I2C2; }
    GPIO_TypeDef* sdaPort() const { return number == 1 ? GPIOB : GPIOA; }
    uint8_t sclPin() const { return number == 1 ? 9 : 11; }
    uint8_t sdaPin() const { return number == 1 ? 9 : 12; }
    static constexpr uint32_t faults = I2C_ISR_NACKF | I2C_ISR_BERR |
                                        I2C_ISR_ARLO | I2C_ISR_OVR;

    bool wait(uint32_t flag, uint32_t start) {
        while (!(regs()->ISR & flag)) {
            if ((regs()->ISR & faults) || uint32_t(millis() - start) >= 25)
                return false;
        }
        return !(regs()->ISR & faults);
    }
    bool clockHigh() {
        GPIOA->BSRR = 1u << sclPin();
        const uint32_t start = millis();
        while (!(GPIOA->IDR & (1u << sclPin())))
            if (uint32_t(millis() - start) >= 2) return false;
        microDelay(5);
        return true;
    }
    bool initialize() {
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
        RCC->APBENR1 |= number == 1 ? RCC_APBENR1_I2C1EN : RCC_APBENR1_I2C2EN;
        regs()->CR1 = 0;
        pinMode(GPIOA, sclPin(), 1);
        pinMode(sdaPort(), sdaPin(), 1);
        if (!clockHigh()) return false;
        for (uint8_t i = 0; i < 9 && !(sdaPort()->IDR & (1u << sdaPin())); ++i) {
            GPIOA->BSRR = 1u << (sclPin() + 16);
            microDelay(5);
            if (!clockHigh()) return false;
        }
        // STOP after clearing an interrupted slave transmission.
        GPIOA->BSRR = 1u << (sclPin() + 16);
        sdaPort()->BSRR = 1u << (sdaPin() + 16);
        microDelay(5);
        if (!clockHigh()) return false;
        sdaPort()->BSRR = 1u << sdaPin();
        microDelay(5);
        if (!(sdaPort()->IDR & (1u << sdaPin()))) return false;
        pinMode(GPIOA, sclPin(), 2, 6);
        pinMode(sdaPort(), sdaPin(), 2, 6);
        // 16 MHz kernel, standard mode: PRESC=3, SCLDEL=4, SDADEL=2,
        // SCLH=15, SCLL=19. External pull-ups; analog filter enabled.
        regs()->TIMINGR = 0x30420f13;
        regs()->ICR = 0xffffffff;
        regs()->CR1 = I2C_CR1_PE;
        initialized = true;
        return true;
    }
    bool transfer(uint8_t address, uint8_t* data, uint8_t length, bool read) {
        if (!initialized && !initialize()) return false;
        const uint32_t start = millis();
        while (regs()->ISR & I2C_ISR_BUSY) {
            if (uint32_t(millis() - start) >= 25) return failed();
        }
        regs()->ICR = 0xffffffff;
        regs()->CR2 = (uint32_t(address) << 1) | (uint32_t(length) << 16) |
                      I2C_CR2_AUTOEND | I2C_CR2_START | (read ? I2C_CR2_RD_WRN : 0);
        for (uint8_t i = 0; i < length; ++i) {
            if (!wait(read ? I2C_ISR_RXNE : I2C_ISR_TXIS, start)) return failed();
            if (read) data[i] = regs()->RXDR;
            else regs()->TXDR = data[i];
        }
        if (!wait(I2C_ISR_STOPF, start)) return failed();
        regs()->ICR = I2C_ICR_STOPCF;
        // STOPF may precede release of BUSY: never disable a live controller.
        while (regs()->ISR & I2C_ISR_BUSY)
            if (uint32_t(millis() - start) >= 25) return failed();
        microDelay(5);
        return true;
    }
    bool failed() {
        regs()->CR1 = 0;
        initialized = false;
        return false;
    }
public:
    explicit I2cBus(uint8_t bus) : number(bus) {}
    static uint32_t now() { return millis(); }
    bool write(uint8_t address, const uint8_t* data, uint8_t length) {
        return transfer(address, const_cast<uint8_t*>(data), length, false);
    }
    bool read(uint8_t address, uint8_t* data, uint8_t length) {
        return transfer(address, data, length, true);
    }
};
inline I2cBus& bus(uint8_t number) {
    static I2cBus first(1), second(2);
    return number == 1 ? first : second;
}
}
