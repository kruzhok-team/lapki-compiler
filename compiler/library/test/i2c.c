#pragma once

// #include <string.h>
// #include <stdio.h>
#include "lib/bases.c"
#include "lib/helpers.c"
#include "usb/status.c"

#define ADDR_EEPROM 0b1010000
#define ADDR_ACCEL 0b0011001
#define EEPROM_OFFSET 32

#define I2C_DATA_SIZE 16

volatile bool i2cWriteComplete = true;
volatile bool i2cReadComplete = true;

uint8_t i2cWriteData[I2C_DATA_SIZE];
uint8_t i2cReadData[I2C_DATA_SIZE];

__attribute__((optimize("O0"))) bool
isAlive(void)
{
    bool result = ((I2C1->ISR) & I2C_ISR_NACKF) == 0;
    I2C1->ICR |= I2C_ICR_NACKCF;
    return result;
}

__attribute__((optimize("O0"))) void
waitBus(void)
{
    // delay(10);
    while (((*(volatile uint8_t *)&(I2C1->ISR)) & I2C_ISR_BUSY) != 0)
        ;
}

void clearData(void)
{
    for (uint8_t i = 0; i < I2C_DATA_SIZE; i++)
    {
        i2cWriteData[i] = 0;
        i2cReadData[i] = 0;
    }
}

void clearInput(void)
{
    for (uint8_t i = 0; i < I2C_DATA_SIZE; i++)
    {
        i2cReadData[i] = 0;
    }
}

void setDmaWrite(uint8_t size)
{
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    DMA1_Channel2->CNDTR = size;
    DMA1_Channel2->CMAR = (uint32_t)(&i2cWriteData);
    DMA1_Channel2->CPAR = (uint32_t)(&I2C1->TXDR);
    DMA1_Channel2->CCR |= DMA_CCR_DIR;
    DMAMUX1_Channel1->CCR &= ~DMAMUX_CxCR_DMAREQ_ID_Msk;
    DMAMUX1_Channel1->CCR |= (17 << DMAMUX_CxCR_DMAREQ_ID_Pos);
    DMA1_Channel2->CCR |= DMA_CCR_EN;
}

void setDmaRead(uint8_t size)
{
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CNDTR = size;
    DMA1_Channel3->CMAR = (uint32_t)(&i2cReadData);
    DMA1_Channel3->CPAR = (uint32_t)(&I2C1->RXDR);
    DMA1_Channel3->CCR &= ~DMA_CCR_DIR;
    DMAMUX1_Channel2->CCR &= ~DMAMUX_CxCR_DMAREQ_ID_Msk;
    DMAMUX1_Channel2->CCR |= (16 << DMAMUX_CxCR_DMAREQ_ID_Pos);
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void initDMA_I2C(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;
    DMA1_Channel2->CMAR = (uint32_t)(&i2cWriteData);
    DMA1_Channel3->CMAR = (uint32_t)(&i2cReadData);

    DMA1_Channel2->CPAR = (uint32_t)(&I2C1->TXDR);
    DMA1_Channel3->CPAR = (uint32_t)(&I2C1->RXDR);

    DMA1_Channel2->CNDTR = 0;
    DMA1_Channel3->CNDTR = 0;

    DMA1_Channel2->CCR |= (DMA_CCR_DIR) | (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos) | (DMA_CCR_MINC);
    DMA1_Channel3->CCR |= (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos) | (DMA_CCR_MINC);

    I2C1->CR1 |= I2C_CR1_TXDMAEN;
    I2C1->CR1 |= I2C_CR1_RXDMAEN;
    DMA1_Channel2->CCR |= DMA_CCR_TCIE;
    DMA1_Channel3->CCR |= DMA_CCR_TCIE;

    DMAMUX1_Channel1->CCR &= ~DMAMUX_CxCR_DMAREQ_ID_Msk;
    DMAMUX1_Channel1->CCR |= (17 << DMAMUX_CxCR_DMAREQ_ID_Pos);
    DMAMUX1_Channel2->CCR &= ~DMAMUX_CxCR_DMAREQ_ID_Msk;
    DMAMUX1_Channel2->CCR |= (16 << DMAMUX_CxCR_DMAREQ_ID_Pos);

    DMA1_Channel2->CCR |= DMA_CCR_EN;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}

void DMA1_Channel2_IRQHandler(void)
{
    DMA1->IFCR |= DMA_IFCR_CTCIF2;
    DMA1->IFCR |= DMA_IFCR_CHTIF2;
    DMA1->IFCR |= DMA_IFCR_CGIF2;
    i2cWriteComplete = true;
}

void DMA1_Channel3_IRQHandler(void)
{
    DMA1->IFCR |= DMA_IFCR_CTCIF3;
    DMA1->IFCR |= DMA_IFCR_CHTIF3;
    DMA1->IFCR |= DMA_IFCR_CGIF3;
    i2cReadComplete = true;
}

void i2cInit(void)
{
    clearData();

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    initPin_AF_OD(GPIOB, 7, 4);  // SCL
    initPin_AF_OD(GPIOA, 15, 4); // SDA
    GPIOB->PUPDR |= (0b01 << GPIO_PUPDR_PUPD7_Pos);
    GPIOA->PUPDR |= (0b01 << GPIO_PUPDR_PUPD15_Pos);

    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    RCC->CCIPR |= (0b10 << RCC_CCIPR_I2C1SEL_Pos);
    delay(2);
    initDMA_I2C();
    // I2C1 -> TIMINGR = 0x0010061A;
    I2C1->TIMINGR = 0x00403D5B; // Explain me, s25.4.10 p.726 RM0454
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR1 |= I2C_CR1_PE;
}

// RM pg 735
uint8_t i2cWrite(uint8_t addr, uint8_t size)
{
    I2C1->CR2 &= ~I2C_CR2_START;
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (size << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;
    I2C1->CR2 &= ~I2C_CR2_SADD_Msk;
    I2C1->CR2 |= (((addr & 0x7f) << 1) << I2C_CR2_SADD_Pos);
    DMA1->IFCR |= DMA_IFCR_CTCIF2;
    setDmaWrite(size);
    I2C1->CR2 |= I2C_CR2_START;
    // while ((( DMA1 -> ISR ) & DMA_ISR_TCIF1 ) == 0 );
    return 0;
}

__attribute__((optimize("O0")))
uint8_t
i2cRead(uint8_t addr, uint8_t size)
{
    clearInput();
    I2C1->CR2 &= ~I2C_CR2_START;
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C1->CR2 |= (size << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_RD_WRN;
    I2C1->CR2 &= ~I2C_CR2_SADD_Msk;
    I2C1->CR2 |= (((addr & 0x7f) << 1) << I2C_CR2_SADD_Pos);
    DMA1->IFCR |= DMA_IFCR_CTCIF3;
    setDmaRead(size);
    I2C1->CR2 |= I2C_CR2_START;
    // while ((( DMA1 -> ISR ) & DMA_ISR_TCIF2 ) == 0 );
    return 0;
}

__attribute__((optimize("O0"))) bool
eepromWrite1(uint8_t addr, uint8_t value)
{
    // while ( !i2cWriteComplete );
    i2cWriteComplete = false;
    i2cWriteData[0] = addr + EEPROM_OFFSET;
    i2cWriteData[1] = value;
    i2cWrite(ADDR_EEPROM, 2);
    while (!i2cWriteComplete)
        ;
    bool ok = isAlive();
    if (!ok)
        flag_failEEPROM();
    return ok;
    // eyeR.color = &ColorGreen;
}

__attribute__((optimize("O0")))
uint8_t
eepromRead1(uint8_t addr)
{
    // while ( !i2cWriteComplete );
    // while ( !i2cReadComplete );
    i2cReadComplete = false;
    i2cWriteComplete = false;
    i2cWriteData[0] = addr + EEPROM_OFFSET;
    i2cWrite(ADDR_EEPROM, 1);
    while (!i2cWriteComplete)
        ;
    i2cRead(ADDR_EEPROM, 1);
    while (!i2cReadComplete)
        ;
    if (!isAlive())
        flag_failEEPROM();
    return i2cReadData[0];
}

__attribute__((optimize("O0"))) void
accelWrite1(uint8_t addr, uint8_t value)
{
    i2cWriteComplete = false;
    i2cWriteData[0] = addr;
    i2cWriteData[1] = value;
    i2cWrite(ADDR_ACCEL, 2);
    if (!isAlive())
        flag_failAccel();
    // eyeR.color = &ColorGreen;
}

__attribute__((optimize("O0")))
uint8_t
accelRead1(uint8_t addr)
{
    // while ( !i2cWriteComplete );
    // while ( !i2cReadComplete );
    i2cReadComplete = false;
    i2cWriteComplete = false;
    i2cWriteData[0] = addr;
    i2cWrite(ADDR_ACCEL, 1);
    while (!i2cWriteComplete)
        ;
    i2cReadData[0] = 0xff;
    i2cRead(ADDR_ACCEL, 1);
    while (!i2cReadComplete)
        ;
    return i2cReadData[0];
}

__attribute__((optimize("O0")))
uint16_t
accelRead2(uint8_t addr)
{
    // while ( !i2cWriteComplete );
    // while ( !i2cReadComplete );
    i2cReadComplete = false;
    i2cWriteComplete = false;
    i2cWriteData[0] = addr;
    i2cWrite(ADDR_ACCEL, 1);
    // delay(5);
    while (!i2cWriteComplete)
        ;
    i2cReadData[0] = 0xff;
    i2cReadData[1] = 0xff;
    i2cRead(ADDR_ACCEL, 2);
    // delay(5);
    while (!i2cReadComplete)
        ;
    uint16_t result = i2cReadData[0];
    result |= (uint16_t)i2cReadData[1] << 8;
    if (!isAlive())
        flag_failAccel();
    return result;
}

__attribute__((optimize("O0"))) void
accelReadN(uint8_t addr, uint8_t len)
{
    // while ( !i2cWriteComplete );
    // while ( !i2cReadComplete );
    i2cReadComplete = false;
    i2cWriteComplete = false;
    i2cWriteData[0] = addr | 0b10000000;
    i2cWrite(ADDR_ACCEL, 1);
    while (!i2cWriteComplete)
        ;
    i2cRead(ADDR_ACCEL, len);
    while (!i2cReadComplete)
        ;
    if (!isAlive())
        flag_failAccel();
}
