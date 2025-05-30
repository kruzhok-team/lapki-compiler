/*
 * gd_uart.cpp
 *
 *  Created on: 14 июл. 2023 г.
 *      Author: laurelindo
 */

#include "gd_uart.h"
#include "gd_lib.h"
#include "yartos.h"

#define UART_DMA_TX_MODE    (DMA_PRIO_LOW    | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | DMA_MEM_INC | DMA_DIR_MEM2PER | DMA_TCIE)
#define UART_DMA_RX_MODE    (DMA_PRIO_MEDIUM | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | DMA_MEM_INC | DMA_DIR_PER2MEM | DMA_CIRC)

#if 1 // ==== TX DMA IRQ ====
// Wrapper for TX IRQ
void UartDmaTxIrqHandler(void *p, uint32_t flags) { ((BaseUart_t*)p)->IRQDmaTxHandler(); }

void BaseUart_t::IRQDmaTxHandler() {
    dma_tx.Disable(); // Registers may be changed only when stream is disabled
    IFullSlotsCount -= ITransSize;
    PRead += ITransSize;
    if(PRead >= (TXBuf + UART_TXBUF_SZ)) PRead = TXBuf; // Circulate pointer
    if(IFullSlotsCount == 0) ITxDmaIsIdle = true; // Nothing left to send
    else ISendViaDMA();
}
#endif

#if 1 // ==== TX / RX ====
void BaseUart_t::ISendViaDMA() {
    uint32_t PartSz = (TXBuf + UART_TXBUF_SZ) - PRead; // Cnt from PRead to end of buf
    ITransSize = MIN_(IFullSlotsCount, PartSz);
    if(ITransSize != 0) {
        ITxDmaIsIdle = false;
        dma_tx.SetMemoryAddr(PRead);
        dma_tx.SetTransferDataCnt(ITransSize);
        params->Uart->STAT0 &= ~USART_STAT0_TC; // Clear TC flag
        dma_tx.Enable();
    }
}

retv BaseUart_t::IPutByteNow(uint8_t b) {
    while(!(params->Uart->STAT0 & USART_STAT0_TBE));
    params->Uart->DATA = b;
    while(!(params->Uart->STAT0 & USART_STAT0_TBE));
    return retv::Ok;
}

retv BaseUart_t::IPutByte(uint8_t b) {
    if(IFullSlotsCount >= UART_TXBUF_SZ) return retv::Overflow;
    *PWrite++ = b;
    if(PWrite >= &TXBuf[UART_TXBUF_SZ]) PWrite = TXBuf;   // Circulate buffer
    IFullSlotsCount++;
    return retv::Ok;
}

void BaseUart_t::IStartTransmissionIfNotYet() {
    if(ITxDmaIsIdle) ISendViaDMA();
}

retv BaseUart_t::GetByte(uint8_t *b) {
    int32_t WIndx = UART_RXBUF_SZ - dma_rx.GetTransferDataCnt();
    int32_t BytesCnt = WIndx - RIndx;
    if(BytesCnt < 0) BytesCnt += UART_RXBUF_SZ;
    if(BytesCnt == 0) return retv::Empty;
    *b = IRxBuf[RIndx++];
    if(RIndx >= UART_RXBUF_SZ) RIndx = 0;
    return retv::Ok;
}
#endif

void BaseUart_t::Init() {
    RCU->EnUart(params->Uart); // Clock
    OnClkChange();  // Setup baudrate

    // ==== TX ====
    Gpio::SetupAlterFunc(params->PGpioTx, params->PinTx, Gpio::PushPull, Gpio::speed50MHz);
    dma_tx.Init(&params->Uart->DATA, UART_DMA_TX_MODE);
    ITxDmaIsIdle = true;

    // ==== RX ====
    Gpio::SetupInput(params->PGpioRx, params->PinRx, Gpio::PullUp);
    dma_rx.Init(&params->Uart->DATA, IRxBuf, UART_DMA_RX_MODE, UART_RXBUF_SZ);
    dma_rx.Enable();

    // UART Regs setup
    params->Uart->CTL0 = USART_CTL0_TEN | USART_CTL0_REN;     // TX & RX en, 8bit, no parity
    params->Uart->CTL1 = 0;  // Nothing interesting there
    params->Uart->CTL2 = USART_CTL2_DENT | USART_CTL2_DENR; // Enable DMA at TX & RX
    params->Uart->CTL0 |= USART_CTL0_UEN;    // Enable USART
}

void BaseUart_t::OnClkChange() {
    if(params->Uart == USART0)
        params->Uart->BAUD = Clk::APB2FreqHz / params->baudrate; // The only UART on APB2
    else
        params->Uart->BAUD = Clk::APB1FreqHz / params->baudrate; // Others are on APB1
}

void BaseUart_t::StopTx() {
    params->Uart->CTL0 &= ~USART_CTL0_UEN; // Disable UART
    dma_tx.DisableAndClearIRQ();
    ITxDmaIsIdle = true;
    PRead = TXBuf;
    PWrite = TXBuf;
    params->Uart->CTL0 |= USART_CTL0_UEN;
}
