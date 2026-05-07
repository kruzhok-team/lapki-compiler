/*
 * SpiFlash.cpp
 *
 *  Created on: 26 окт. 2023 г.
 *      Author: laurelindo
 */

#include "SpiFlash.h"
#include "yartos.h"
#include "shell.h"

// DMA
#define DMA_RX_MODE DMA_PRIO_HIGH | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | \
    DMA_MEM_INC | DMA_DIR_PER2MEM | DMA_TCIE

// Mode for reading, i.e. no irq, no mem inc (as it is required to send zeroes only)
#define DMA_TX_MODE_NOIRQ_NOINC DMA_PRIO_HIGH | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | DMA_DIR_MEM2PER
// Mode for writing
#define DMA_TX_MODE_IRQ_INC DMA_PRIO_HIGH | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | \
    DMA_MEM_INC | DMA_DIR_MEM2PER | DMA_TCIE

void SpiFlashDmaCb(void *p, uint32_t dw32) {
    Sys::LockFromIRQ();
    Sys::WakeI(&((SpiFlash*)p)->pthd, retv::Ok);
    Sys::UnlockFromIRQ();
}

SpiFlash::SpiFlash(SPI_TypeDef *pspi) : spi(pspi),
        dma_tx(SPIFLASH_DMA_TX, SpiFlashDmaCb, this),
        dma_rx(SPIFLASH_DMA_RX, SpiFlashDmaCb, this)
        {}

void SpiFlash::Init() {
    // Init GPIO
    nss_pin.SetupOut(Gpio::PushPull, Gpio::speed50MHz);
    nss_pin.SetHi();
    Gpio::SetupAlterFunc(FLASH_SCK,  Gpio::PushPull, Gpio::speed50MHz);
    Gpio::SetupAlterFunc(FLASH_MISO, Gpio::PushPull, Gpio::speed50MHz);
    Gpio::SetupAlterFunc(FLASH_MOSI, Gpio::PushPull, Gpio::speed50MHz);
    Gpio::SetupAlterFunc(FLASH_IO2,  Gpio::PushPull, Gpio::speed50MHz);
    Gpio::SetupAlterFunc(FLASH_IO3,  Gpio::PushPull, Gpio::speed50MHz);
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, BitNum = 8
    spi.Setup(BitOrder::MSB, SpiHw::Cpol::IdleLow, SpiHw::Cpha::FirstEdge, SPIFLASH_CLK_FREQ_Hz);
//    spi.Setup(BitOrder::MSB, SpiHw::Cpol::IdleLow, SpiHw::Cpha::FirstEdge, 1000000);
    spi.Enable();
    // ==== DMA ====
    dma_rx.Init();
    dma_rx.SetPeriphAddr(&spi.PSpi->DATA);
    dma_rx.SetMode(DMA_RX_MODE);
    dma_tx.Init();
    dma_tx.SetPeriphAddr(&spi.PSpi->DATA);
}

SpiFlash::MemParams SpiFlash::GetParams() {
    MemParams r;
    // Read JEDEC ID
    uint32_t JID;
    nss_pin.SetLo();
    spi.WriteRead(0x9F); // cmd code Read JEDEC ID
    JID = spi.WriteRead(0x00); // MfgId
    JID = (JID << 8) | spi.WriteRead(0x00); // MemType
    JID = (JID << 8) | spi.WriteRead(0x00); // Capacity
    nss_pin.SetHi();
//    Printf("JID: 0x%X\r", JID);
    switch(JID) {
        case 0xEF4016: // W25Q32
            r.sector_cnt = 1024UL;
            r.sector_sz = 4096UL;
            break;
        case 0xEF4017: // W25Q64
            r.sector_cnt = 2048UL;
            r.sector_sz = 4096UL;
            break;
        default: Printf("Unknown Flash JID: 0x%X\r", JID); break;
    } // switch
    return r;
}

#if 1 // ======================= Read / Write / Erase ==========================
retv SpiFlash::Read(uint32_t addr, uint8_t *pbuf, uint32_t alen) {
    nss_pin.SetLo();
    WriteCmdAndAddr(0x0B, addr); // cmd FastRead
    spi.WriteRead(0x00); // 8 dummy clocks
    // ==== Read Data ====
    spi.ClearRxBuf();
    spi.Disable();
    spi.SetRxOnly();   // Will not set if enabled
    spi.EnRxDma();
    dma_rx.SetMemoryAddr(pbuf);
    dma_rx.SetTransferDataCnt(alen);
    // Start
    Sys::Lock();
    pthd = Sys::GetSelfThd();
    dma_rx.Enable();
    spi.Enable();
    retv r = Sys::SleepS(TIME_MS2I(270)); // Wait IRQ
    // Will be here after timeout or IRQ
    dma_rx.Disable();
    Sys::Unlock();
    nss_pin.SetHi();
    spi.Disable();
    spi.SetFullDuplex();   // Remove read-only mode
    spi.DisRxDma();
    spi.Enable();
    spi.PSpi->WaitForTBEHiAndTransLo(); // }
    spi.ClearRxBuf();                   // } Clear bufs and flags
    return r;
}

retv SpiFlash::ReadQ(uint32_t addr, uint8_t *pbuf, uint32_t alen) {
    nss_pin.SetLo();
    spi.WriteRead(0xEB); // Write cmd in single mode
    spi.ClearRxBuf();
    spi.EnQuadWrite();
    // Send addr and 2 dummy clocks
    spi.WriteRead(0xFF & (addr >> 16));
    spi.WriteRead(0xFF & (addr >> 8));
    spi.WriteRead(0xFF & (addr >> 0));
    spi.WriteRead(0xF0); // 2 dummy clocks, must be Fx (see memory datasheet)
    // Switch to read mode
    spi.EnQuadRead();
    // Send 4 more clocks
    spi.Write(0x00);
    spi.Write(0x00);
    // ==== Read Data ====
    dma_rx.SetMemoryAddr(pbuf);
    dma_rx.SetTransferDataCnt(alen);
    uint8_t Dummy = 0;
    dma_tx.SetMemoryAddr(&Dummy);
    dma_tx.SetTransferDataCnt(alen);
    dma_tx.SetMode(DMA_TX_MODE_NOIRQ_NOINC);
    spi.PSpi->WaitForTBEHiAndTransLo();
    spi.ClearRxBuf();
    spi.EnRxTxDma();
    // Start
    Sys::Lock();
    pthd = Sys::GetSelfThd();
    dma_rx.Enable();
    dma_tx.Enable();
    retv r = Sys::SleepS(TIME_MS2I(270)); // Wait IRQ
    // Will be here after timeout or IRQ
    dma_rx.Disable();
    dma_tx.Disable();
    Sys::Unlock();
    nss_pin.SetHi();
    spi.DisRxTxDma();
    spi.PSpi->WaitForTBEHiAndTransLo(); // }
    spi.ClearRxBuf();                   // } Clear bufs and flags
    spi.DisQuad();
    return r;
}


retv SpiFlash::WritePage(uint32_t addr, uint8_t *pbuf, uint32_t alen) {
    WriteEnable();
    nss_pin.SetLo();
    WriteCmdAndAddr(0x02, addr);
    // Write data
    for(uint32_t i=0; i < alen; i++) spi.WriteRead(*pbuf++);
    nss_pin.SetHi();
    return BusyWait(); // Wait completion
}

retv SpiFlash::WritePageQ(uint32_t addr, uint8_t *pbuf, uint32_t alen) {
    WriteEnable();
    nss_pin.SetLo();
    spi.WriteRead(0x32); // Write cmd and addr in single mode
    // Send addr and 2 dummy clocks
    spi.WriteRead(0xFF & (addr >> 16));
    spi.WriteRead(0xFF & (addr >> 8));
    spi.WriteRead(0xFF & (addr >> 0));
    // Write data
    dma_tx.SetMemoryAddr(pbuf);
    dma_tx.SetTransferDataCnt(alen);
    dma_tx.SetMode(DMA_TX_MODE_IRQ_INC);
    spi.EnQuadWrite();
    spi.EnTxDma();
    Sys::Lock();
    pthd = Sys::GetSelfThd();
    dma_tx.Enable();
    retv r = Sys::SleepS(TIME_MS2I(270)); // Wait IRQ
    // Will be here after timeout or IRQ
    dma_tx.Disable();
    Sys::Unlock();
    spi.PSpi->WaitForTBEHiAndTransLo(); // }
    spi.ClearRxBuf();                   // } Clear bufs and flags
    nss_pin.SetHi();
    spi.DisTxDma();
    spi.DisQuad();
    if(r == retv::Ok) return BusyWait(); // Wait completion
    else return r;
}

retv SpiFlash::EraseSector4k(uint32_t addr) {
    WriteEnable();
    nss_pin.SetLo();
    WriteCmdAndAddr(0x20, addr);
    nss_pin.SetHi();
    return BusyWait(); // Wait completion
}

retv SpiFlash::EraseBlock32k(uint32_t addr) {
    WriteEnable();
    nss_pin.SetLo();
    WriteCmdAndAddr(0x52, addr);
    nss_pin.SetHi();
    return BusyWait(); // Wait completion
}

retv SpiFlash::EraseBlock64k(uint32_t addr) {
    WriteEnable();
    nss_pin.SetLo();
    WriteCmdAndAddr(0xD8, addr);
    nss_pin.SetHi();
    return BusyWait(); // Wait completion
}
#endif

// ========================= Control Instructions ==============================
uint8_t SpiFlash::ReleasePowerDown() {
    nss_pin.SetLo();
    spi.WriteRead(0xAB); // cmd code
    spi.WriteRead(0x00); // }
    spi.WriteRead(0x00); // }
    spi.WriteRead(0x00); // } Three dummy bytes
    uint8_t r = spi.WriteRead(0x00);
    nss_pin.SetHi();
    return r;
}


/* UNUSED, left for dbg purposes
SpiFlash::MfrDevId_t SpiFlash::ReadMfrDevId() {
    MfrDevId_t r;
    Nss.SetLo();
    spi.WriteRead(0x90);
    spi.WriteRead(0x00);
    spi.WriteRead(0x00);
    spi.WriteRead(0x00);
    r.Mfr = spi.WriteRead(0x00);
    r.DevID = spi.WriteRead(0x00);
    Nss.SetHi();
    return r;
}

SpiFlash::MfrDevId_t SpiFlash::ReadMfrDevIdQ() {
    MfrDevId_t r;
    Nss.SetLo();
    spi.WriteRead(0x94); // Write cmd in single mode
    spi.EnQuadWrite();
    // Send 8 clocks
    spi.Write(0x00);
    spi.Write(0x00);
    spi.Write(0x00);
    spi.Write(0x00);
    // Switch to read mode
    spi.EnQuadRead();
    // Send 4 more clocks
    spi.Write(0x00);
    spi.Write(0x00);
    spi.PSpi->WaitForTBEHiAndTransLo();
    spi.ClearRxBuf();
    // Read payload
    r.Mfr = spi.WriteRead(0x00);
    r.DevID = spi.WriteRead(0x00);
    Nss.SetHi();
    spi.DisQuad();
    return r;
}
*/

#if 1 // ========================== Service ====================================
void SpiFlash::Reset() {
    // Software Reset sequence: Enable Reset (66h) & Reset (99h).
    Sys::Lock();
    nss_pin.SetLo();
    spi.WriteRead(0x66); // Enable Reset
    nss_pin.SetHi();
    __NOP(); __NOP(); __NOP(); __NOP();
    nss_pin.SetLo();
    spi.WriteRead(0x99); // Do Reset
    nss_pin.SetHi();
    Sys::Unlock();
    Sys::SleepMilliseconds(1); // device will take approximately 30μS (tRST) to reset
}

void SpiFlash::WriteCmdAndAddr(uint8_t cmd, uint32_t addr) {
    spi.WriteRead(cmd);
    spi.WriteRead(0xFF & (addr >> 16));
    spi.WriteRead(0xFF & (addr >> 8));
    spi.WriteRead(0xFF & (addr >> 0));
}

// Write cmd and then read next byte
uint8_t SpiFlash::WriteCmdRead1Byte(uint8_t cmd) {
    nss_pin.SetLo();
    spi.WriteRead(cmd);
    uint8_t r = spi.WriteRead(0x00);
    nss_pin.SetHi();
    return r;
}

void SpiFlash::WriteEnable() {
    nss_pin.SetLo();
    spi.WriteRead(0x06);
    nss_pin.SetHi();
}

retv SpiFlash::BusyWait() {
    Sys::SleepMilliseconds(1);
    systime_t Start = Sys::GetSysTimeX();
    retv r = retv::Timeout;
    nss_pin.SetLo();
    spi.WriteRead(0x05); // Read StatusReg1
    while(Sys::TimeElapsedSince(Start) < TIME_MS2I(SPIFLASH_TIMEOUT_ms)) {
        Sys::SleepMilliseconds(1);
        uint8_t b = spi.WriteRead(0);
//        Printf("%X\r", r);
        if((b & 0x01) == 0) { // BUSY bit == 0
            r = retv::Ok;
            break;
        }
    }
    nss_pin.SetHi();
    return r;
}
#endif
