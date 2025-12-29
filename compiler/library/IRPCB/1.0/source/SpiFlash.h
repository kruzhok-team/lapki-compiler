/*
 * SpiFlash.h
 *
 *  Created on: 26 окт. 2023 г.
 *      Author: laurelindo
 */

#ifndef LIB_SPIFLASH_H_
#define LIB_SPIFLASH_H_

#include "gd32e11x_kl.h"
#include "gd_lib.h"
#include "yartos.h"

/* ReadData allows 50Mhz only, so it is not implemented. Read method uses
 * Fast Read instead. For voltages below 3v0, set 104MHz. */
//#define SPIFLASH_CLK_FREQ_Hz    133000000UL // For voltages 3v0...3v6
//#define SPIFLASH_CLK_FREQ_Hz    1000000UL // Do it unhurriedly
#define SPIFLASH_CLK_FREQ_Hz    104000000UL // For voltages 2v7...3v0
#define SPIFLASH_PAGE_SZ        256UL    // Defined in datasheet
#define SPIFLASH_BLOCK32_SZ     32768UL  // 128 pages
#define SPIFLASH_BLOCK64_SZ     65536UL  // 256 pages

#define SPIFLASH_TIMEOUT_ms     999UL

class SpiFlash {
private:
    SpiHw spi;
    Pin_t nss_pin{FLASH_NSS};
    DMA_t dma_tx, dma_rx;
    Thread* pthd = nullptr;
    uint8_t WriteCmdRead1Byte(uint8_t cmd);
    void WriteCmdAndAddr(uint8_t cmd, uint32_t addr);
    void WriteEnable();
    retv BusyWait();
    friend void SpiFlashDmaCb(void *p, uint32_t dw32);
public:
    SpiFlash(SPI_TypeDef *pspi);
    void Init();

    struct MemParams {
        uint32_t sector_cnt = 0, sector_sz = 0;
    };

    MemParams GetParams();

    // ==== Instructions ====
    // Read / Write / Erase
    retv Read(uint32_t addr, uint8_t *pbuf, uint32_t alen);
    retv ReadQ(uint32_t addr, uint8_t *pbuf, uint32_t alen);

    retv WritePage(uint32_t addr, uint8_t *pbuf, uint32_t alen);
    retv WritePageQ(uint32_t addr, uint8_t *pbuf, uint32_t alen);

    retv EraseSector4k(uint32_t addr);
    retv EraseBlock32k(uint32_t addr);
    retv EraseBlock64k(uint32_t addr);
    retv EraseAndWriteSector4k(uint32_t addr, uint8_t *pbuf);

    // Control
    void Reset();
    uint8_t ReleasePowerDown(); // Returns ID[7:0]

    // Status Regs
    uint8_t ReadStatusReg1() { return WriteCmdRead1Byte(0x05); }
    uint8_t ReadStatusReg2() { return WriteCmdRead1Byte(0x35); }
    uint8_t ReadStatusReg3() { return WriteCmdRead1Byte(0x15); }
//    void WriteStatusReg1(uint8_t b);
//    void WriteStatusReg2(uint8_t b);
//    void WriteStatusReg3(uint8_t b);

    void PowerDown();
};

#endif /* LIB_SPIFLASH_H_ */
