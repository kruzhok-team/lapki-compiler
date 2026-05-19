/*
 * ir.cpp
 *
 *  Created on: 04.07.2013
 *      Author: kreyl
 */

#include "ir.h"
#include "gd_uart.h"
#include "Settings.h"

#if IR_TX_ENABLED // ========================== IR TX ==========================


namespace IRLed {
    #define IRLED_DMA_MODE  DMA_PRIO_HIGH | DMA_MEMSZ_8_BIT | DMA_PERSZ_8_BIT | DMA_MEM_INC | DMA_DIR_MEM2PER | DMA_TCIE

    TimHw sampling_tmr{TMR_DAC_SMPL};
    ftVoidVoid icallbackI = nullptr;
    static uint32_t transaction_sz;
    static void DmaTxEndIrqHandler(void *p, uint32_t flags);
    static const DMA_t dma_tx {DAC_DMA, DmaTxEndIrqHandler, nullptr, IRQ_PRIO_MEDIUM};
    // Samples count
    static struct {
        int32_t header, space, one, zero, pause;
    } samples_cnt;

    // DAC buf: reserve size fo 56kHz (969)
    #define DAC_BUF_SZ          999UL

    union DacSamplePair {
        uint32_t dw32;
        struct {
            uint8_t on_pwr1;
            uint8_t off_pwr1;
            uint8_t on_pwr2;
            uint8_t off_pwr2;
        } __attribute__((packed));
        DacSamplePair& operator = (const DacSamplePair& right) {
            dw32 = right.dw32;
            return *this;
        }
        DacSamplePair() : dw32(0) {}
        DacSamplePair(uint8_t pwr) : on_pwr1(pwr), off_pwr1(0), on_pwr2(pwr), off_pwr2(0) {}
    } __attribute__((packed));

    #define SEQUENCE_LENGTH 14000
    #define SAMPLE_RATE (56000*2)
    #define FS48_TO_FS112(X) int((X + 0.0) * 2.333333 + 0.5)
    
    // Хранить два буфера отдельно мы не можем по памяти,
    // из-за чего мы выделяем один буфер под все и в моменте
    // использования кастим к нужному буферу
    alignas(DacSamplePair) static uint8_t tx_buf_storage[
        (DAC_BUF_SZ * sizeof(DacSamplePair) > SEQUENCE_LENGTH) ? DAC_BUF_SZ * sizeof(DacSamplePair) : SEQUENCE_LENGTH
    ];

    static void DmaTxEndIrqHandler(void *p, uint32_t flags) {
        Sys::LockFromIRQ();
        sampling_tmr.Disable();
        dma_tx.Disable();
        if(icallbackI) icallbackI();
        Sys::UnlockFromIRQ();
    }

    static inline void StartTx() {
        dma_tx.SetMemoryAddr(tx_buf_storage);
        dma_tx.SetTransferDataCnt(transaction_sz);
        dma_tx.Enable();
        sampling_tmr.Enable();
    }

    void Init() {
        // ==== GPIO ====
        // Once the DAC channel is enabled, the corresponding GPIO pin is automatically
        // connected to the DAC converter. In order to avoid parasitic consumption,
        // the GPIO pin should be configured in analog.
        Gpio::SetupAnalog(IR_LED);
        // ==== DAC ====
        RCU->EnDAC();
        DAC->SetTrigger0(DAC_TypeDef::Trigger::Tim6TRGO);
        DAC->EnTrigger0();
        DAC->EnDma0();
        DAC->Enable0();
        // ==== DMA ====
        dma_tx.Init(&DAC->DAC0_R8DH, IRLED_DMA_MODE);
        // ==== Sampling timer ====
        sampling_tmr.Init();
        sampling_tmr.SelectMasterMode(TimHw::MasterMode::Update);
    }

    void SetCarrierFreq(uint32_t carrier_freq_Hz) {
        uint32_t sampling_freq_Hz = carrier_freq_Hz * 2;
        sampling_tmr.SetUpdateFreqChangingTopValue(sampling_freq_Hz);
        // Every SamplePair contains 4 actual samples
        samples_cnt.header = (((kIr::Header_us * carrier_freq_Hz) + 1999999L) / 2000000L);
        samples_cnt.space  = (((kIr::Space_us  * carrier_freq_Hz) + 1999999L) / 2000000L);
        samples_cnt.zero   = (((kIr::Zero_us   * carrier_freq_Hz) + 1999999L) / 2000000L);
        samples_cnt.one    = (((kIr::One_us    * carrier_freq_Hz) + 1999999L) / 2000000L);
        samples_cnt.pause  = (((kIr::PauseAfter_us * carrier_freq_Hz) + 1999999UL) / 2000000UL);
        // Check if buf sz is enough
        uint32_t N = samples_cnt.header + samples_cnt.space + (samples_cnt.one + samples_cnt.space) * IR_BIT_CNT_MAX + samples_cnt.pause;
        if(N > DAC_BUF_SZ) Printf("IR TX DAC Buf Sz too small: %d < %d\r\n", DAC_BUF_SZ, N);
    }

    // power is 8-bit DAC value
    void TransmitStandardWord(uint16_t data, int32_t bit_cnt, uint8_t power, ftVoidVoid callbackI) {
        icallbackI = callbackI;
        // ==== Fill buffer depending on data ====
        DacSamplePair *p = reinterpret_cast<DacSamplePair*>(tx_buf_storage);
        DacSamplePair sample_carrier{power};
        DacSamplePair sample_space{0};
        int32_t i, j;
        // Put header
        for(i=0; i<samples_cnt.header; i++) *p++ = sample_carrier;
        for(i=0; i<samples_cnt.space; i++)  *p++ = sample_space;
        // Put data
        for(j=0; j<bit_cnt; j++) {
            // Carrier
            if(data & 0x8000) { for(i=0; i<samples_cnt.one;  i++) *p++ = sample_carrier; }
            else               { for(i=0; i<samples_cnt.zero; i++) *p++ = sample_carrier; }
            // space
            for(i=0; i<samples_cnt.space; i++)  *p++ = sample_space;
            data <<= 1;
        }
        // Put pause
        for(i=0; i<samples_cnt.pause; i++) *p++ = sample_space;
        // ==== Start transmission ====
        transaction_sz = (p - reinterpret_cast<DacSamplePair*>(tx_buf_storage)) * 4; // Every sample pair contains 4 actual samples
        StartTx();
    }

    void TransmitCyberBearWord(uint16_t data, int32_t bit_cnt, uint8_t power, ftVoidVoid callbackI) {
        icallbackI = callbackI;

        int period_6000_x1000 = (SAMPLE_RATE * 1000) / 6000;  // 8000
        int period_2730_x1000 = (SAMPLE_RATE * 1000) / 2730;  // ~17582

        for (int i = 0; i < SEQUENCE_LENGTH; i++) {
            uint8_t value = 0;

            if (i < FS48_TO_FS112(64)) {
                int phase = (i * 1000) % period_6000_x1000;
                value = (phase >= period_6000_x1000 / 2) ? 255 : 0;
            }

            for (int k = 0; k < 9; k++) {
                int start = FS48_TO_FS112(135) + FS48_TO_FS112(595) * k;
                if (i >= start && i < start + FS48_TO_FS112(64)) {
                    int j = i - start;
                    int phase = (j * 1000) % period_2730_x1000;
                    uint8_t multiplier = (k == 0) ? 1 : (data & (0x80 >> (k - 1))) ? 1 : 0;
                    value = multiplier * ((phase >= period_2730_x1000 / 2) ? 255 : 0);
                }
            }

            tx_buf_storage[i] = value;
        }

        dma_tx.SetMemoryAddr(tx_buf_storage);
        dma_tx.SetTransferDataCnt(SEQUENCE_LENGTH);
        dma_tx.Enable();
        sampling_tmr.Enable();
    }

    // Эта функция нам нужна, чтобы не менять интерфейс
    // отправки данных в функциях инициализации и при отправке команд 
    void TransmitWord(uint16_t data, int32_t bit_cnt, uint8_t power, ftVoidVoid callbackI) {
        if (settings.tx_mode == CYBERBEAR_MODE) {
            TransmitCyberBearWord(data, bit_cnt, power, callbackI);
        } else {
            TransmitStandardWord(data, bit_cnt, power, callbackI);
        }
    }

    void ResetI() {
        dma_tx.Disable();
        sampling_tmr.Disable();
        DAC->PutDataR0(0);
        sampling_tmr.GenerateUpdateEvt();
    }
} // namespace
#endif

#if IR_RX_ENABLED // ========================== IR RX ==========================
/* ==== Timer ====
Here Input1 is used for resetting on falling edge and capturing on rising one.
Two outputs of EdgeDetector1 are used: CI1FE0 and CI1FE1.
Edge polarity is set in CHCTL2 reg: for CI1FE0 using CH0P bit, and for
CI1FE1 using CH1P bit. (CH0P sets edge polarity for both CI0FE0 and CI1FE0 signals;
CH1P - for CI0FE1 and CI1FE1; and so on).
____        _____________________
    |______|   InterBitTimeot    ^
    ^      ^                     ^
 TI2FP2   TI2FP1                OVF => Timeout IRQ
 CI1FE1   CI1FE0
 Trigger  Capture
 Reset    CCR1 => Capture IRQ
*/
namespace IRRcvr {

ftVoidU8U16 callbackI;
TimHw tmr_rx{TMR_IR_RX};

void Init() {
    Gpio::SetupInput(IR_RX_DATA_PIN, Gpio::PullUp);
    tmr_rx.Init();
    tmr_rx.DisableAutoreloadBuffering(); // To put there timeout immediately
    tmr_rx.SetTopValue(0xFFFF); // Maximum, just to start
    tmr_rx.SetUpdateSrcOvfOnly();
    tmr_rx.SetInputFreqChangingPrescaler(1000000);  // Input Freq: 1 MHz => one tick = 1 uS
    // Setup input mode for Channel0: capture Input1 (not Input0) on rising edge
    tmr_rx.SetChnlMode(0, TimHw::ChnlMode::CI1FE0); // Chnl0 is input, capture on Input1's CI1FE0 signal
    tmr_rx.SetInputActiveEdge(0, RiseFall::Rising); // CI1FE0 is Active Rising (CI0FE0 is the same, but it is not used)
    // Reset timer on trigger; trigger is falling edge on CI1FE1
    tmr_rx.SetTriggerInput(TimHw::TriggerIn::CI1FE1); // Use Input1's CI1FE1 as TriggerIn
    tmr_rx.SetInputActiveEdge(1, RiseFall::Falling);
    // Configure slave mode controller in Restart mode
    tmr_rx.SelectSlaveMode(TimHw::SlaveMode::Restart);
    // Enable the capture on channel 0
    tmr_rx.EnChnl(0);
    // IRQ
    tmr_rx.EnableIrqOnCompare0();
    Nvic::EnableVector(TMR_IR_RX_IRQ, IRQ_PRIO_HIGH);
    // Start capture
    tmr_rx.Enable();
}

// Parsing
static int32_t bit_indx = -1; // header not received
static uint32_t rx_data;

static void OnReceptionDone() {
//    PrintfI("ibto\r");
    int32_t bit_cnt = 16L - bit_indx;
    if(bit_cnt > 0 and bit_cnt <= 16) { // Some bits were received
        if(callbackI) callbackI(bit_cnt, rx_data);
    }
    tmr_rx.DisableIrqOnUpdate(); // Disable interbit timeout
    bit_indx = -1; // Reset reception
}

static void ProcessIrqI(uint32_t flags, uint32_t dur) {
//    PrintfI("0x%X %u\r",  flags, dur);
    if(IsLike<uint32_t>(dur, kIr::Header_us, *settings.ir_rx_deviation)) { // header rcvd
        bit_indx = 16;
        rx_data = 0;
        // Setup timeout
        tmr_rx.SetTopValue(dur + kIr::InterBitTimeot_us);
        tmr_rx.EnableIrqOnUpdate();
    }
    else { // Not header
        // Check if timeout occured, i.e. update IRQ fired
        if(TimHw::IsUpdateFlagSet(flags)) OnReceptionDone();
        else if(bit_indx != -1) { // No timeout; ignore received if not after header, or if bad length is received previously
            uint32_t bit;
            if     (IsLike<uint32_t>(dur, kIr::Zero_us, *settings.ir_rx_deviation)) bit = 0UL;
            else if(IsLike<uint32_t>(dur, kIr::One_us,  *settings.ir_rx_deviation)) bit = 1UL;
            else { // Bad duration
                bit_indx = -1;
                tmr_rx.DisableIrqOnUpdate(); // Disable interbit timeout
                return;
            }
            // Find out expected bit cnt
            bit_indx--;
            rx_data |= bit << bit_indx;
            if(bit_indx == 0) OnReceptionDone(); // Reception completed, as maximim amount of bits received
        }
    }
}

} // namespace

// ==== IRQ ====
extern "C"
void TMR_IR_RX_IRQ_HNDLR() {
    Sys::IrqPrologue();
    Sys::LockFromIRQ();
    uint32_t flags = IRRcvr::tmr_rx.GetIrqFlags();
    IRRcvr::tmr_rx.ClearAllIrqFlags();
    IRRcvr::ProcessIrqI(flags, IRRcvr::tmr_rx.GetChnl0Value());
    Sys::UnlockFromIRQ();
    Sys::IrqEpilogue();
}
#endif
