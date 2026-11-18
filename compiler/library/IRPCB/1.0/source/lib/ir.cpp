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
static uint32_t standard_carrier_freq_Hz = 56000;
static volatile bool tx_busy = false;
static volatile uint32_t last_tx_end_ms = 0;
static volatile bool have_tx_end = false;
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

// One existing 3996-byte allocation, reused as byte samples in CyberBear mode.
DacSamplePair dac_buf[DAC_BUF_SZ];
static constexpr uint32_t kBearSymbolSamples = 256;
// Two quiet symbols let the receiver distinguish a new header after zero bits.
static constexpr uint32_t kBearFrameSamples = 12 * kBearSymbolSamples;
static_assert(sizeof(DacSamplePair) == 4, "Unexpected DAC sample layout");
static_assert(kBearFrameSamples <= sizeof(dac_buf), "CyberBear frame exceeds shared buffer");

static bool ReserveTx() {
    // The buffer is shared by all callers, including the shell and app thread.
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    bool available = !tx_busy;
    if(available) tx_busy = true;
    __set_PRIMASK(mask);
    return available;
}

static void DmaTxEndIrqHandler(void *p, uint32_t flags) {
    // A cleared/pending IRQ after ResetI must not complete a new frame.
    if((flags & 0b0010UL) == 0) return; // DMA transfer-complete flag.
    Sys::LockFromIRQ();
    sampling_tmr.Disable();
    dma_tx.Disable();
    DAC->PutDataR0(0);
    sampling_tmr.GenerateUpdateEvt();
    last_tx_end_ms = Sys::Time_ + Sys::GetSysTimeX();
    have_tx_end = true;
    ftVoidVoid callback = icallbackI;
    icallbackI = nullptr;
    tx_busy = false;
    if(callback) callback();
    Sys::UnlockFromIRQ();
}

static inline void StartTx() {
    dma_tx.DisableAndClearIRQ();
    dma_tx.SetMode(IRLED_DMA_MODE); // DisableAndClearIRQ also clears TCIE.
    dma_tx.SetMemoryAddr(dac_buf);
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
    // Settings support up to 56 kHz, for which the standard frame fits.
    if(carrier_freq_Hz == 0 || carrier_freq_Hz > 56000) return;
    standard_carrier_freq_Hz = carrier_freq_Hz;
    // Do not disturb an active DMA transfer, regardless of its protocol.
    if(!tx_busy) sampling_tmr.SetUpdateFreqChangingTopValue(carrier_freq_Hz * 2);
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
static void TransmitStandardWord(uint16_t data, int32_t bit_cnt, uint8_t power) {
    sampling_tmr.SetUpdateFreqChangingTopValue(standard_carrier_freq_Hz * 2);
    // ==== Fill buffer depending on data ====
    DacSamplePair *p = dac_buf, sample_carrier{power}, sample_space{0};
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
    transaction_sz = (p - dac_buf) * 4; // Every sample pair contains 4 actual samples
    StartTx();
}

static void TransmitCyberBearWord(uint16_t data, uint8_t power) {
    sampling_tmr.SetUpdateFreqChangingTopValue(48000);
    // uint8_t aliases the object representation of the same DacSamplePair array.
    uint8_t *samples = reinterpret_cast<uint8_t*>(dac_buf);
    uint8_t byte = static_cast<uint8_t>(data);
    for(uint32_t t = 0; t < kBearFrameSamples; ++t) {
        uint32_t symbol = t / kBearSymbolSamples;
        uint32_t period = 4; // Header and logical zero: 12 kHz.
        if(symbol >= 2 && symbol < 10 && (byte & (0x80 >> (symbol - 2))))
            period = 32; // Logical one: 1.5 kHz.
        // Symbol 1 is the header gap; symbols 10 and 11 are the inter-frame pause.
        samples[t] = symbol != 1 && symbol < 10 && t % period < period / 2 ? power : 0;
    }
    transaction_sz = kBearFrameSamples;
    StartTx();
}

// Preserve the original Settings dispatcher and the public transmission API.
void TransmitWord(uint16_t data, int32_t bit_cnt, uint8_t power, ftVoidVoid callbackI) {
    if(settings.tx_mode != STANDARD_MODE && settings.tx_mode != CYBERBEAR_MODE) return;
    if(settings.tx_mode == STANDARD_MODE && (bit_cnt < 1 || bit_cnt > IR_BIT_CNT_MAX)) return;
    // A second call is ignored while DMA owns the shared buffer.
    if(!ReserveTx()) return;
    icallbackI = callbackI;
    if(settings.tx_mode == CYBERBEAR_MODE) TransmitCyberBearWord(data, power);
    else TransmitStandardWord(data, bit_cnt, power);
}

void ResetI() {
    if(tx_busy) {
        last_tx_end_ms = Sys::Time_ + Sys::GetSysTimeX();
        have_tx_end = true;
    }
    dma_tx.DisableAndClearIRQ();
    sampling_tmr.Disable();
    DAC->PutDataR0(0);
    sampling_tmr.GenerateUpdateEvt();
    icallbackI = nullptr;
    tx_busy = false;
}

bool IsTransmitting() { return tx_busy; }

bool IsEchoSuppressed(uint32_t now_ms, uint32_t quiet_ms) {
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    bool suppressed = tx_busy ||
        (have_tx_end && static_cast<uint32_t>(now_ms - last_tx_end_ms) < quiet_ms);
    __set_PRIMASK(mask);
    return suppressed;
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
static CyberBearReceiver cyberbear_rx;

static void ConfigureStandard() {
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

// Keep the standard capture decoder and the raw-carrier sampler independent.
void SetMode(uint8_t mode) {
    if(mode != STANDARD_MODE && mode != CYBERBEAR_MODE) return;
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    tmr_rx.Disable();
    tmr_rx.DisableIrqOnCompare0();
    tmr_rx.DisableIrqOnUpdate();
    settings.rx_mode = mode;
    bit_indx = -1;
    rx_data = 0;
    cyberbear_rx = CyberBearReceiver{};
    if(mode == STANDARD_MODE) ConfigureStandard();
    else {
        Gpio::SetupInput(IR_RX_DATA_PIN, Gpio::PullUp);
        tmr_rx.Init();
        // This mode needs the raw carrier at PA1, not a demodulated envelope.
        tmr_rx.SelectSlaveMode(TimHw::SlaveMode::Disable);
        tmr_rx.DisableAutoreloadBuffering();
        tmr_rx.SetUpdateSrcOvfOnly();
        tmr_rx.SetPrescaler(0);
        tmr_rx.SetUpdateFreqChangingTopValue(48000);
        tmr_rx.EnableIrqOnUpdate();
        Nvic::EnableVector(TMR_IR_RX_IRQ, IRQ_PRIO_HIGH);
        tmr_rx.Enable();
    }
    tmr_rx.SetCounter(0);
    tmr_rx.GenerateUpdateEvt();
    tmr_rx.ClearAllIrqFlags();
    __set_PRIMASK(mask);
}

void Init() { SetMode(settings.rx_mode); }

CyberBearReceiver::Diagnostics GetDiagnostics() {
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    CyberBearReceiver::Diagnostics diagnostics = cyberbear_rx.GetDiagnostics();
    __set_PRIMASK(mask);
    return diagnostics;
}

static void ProcessCyberBearSample(bool level) {
    uint8_t byte;
    bool muted = false;
#if IR_TX_ENABLED
    muted = IRLed::IsTransmitting();
#endif
    if(cyberbear_rx.Sample(level, byte, muted) && callbackI)
        callbackI(8, byte);
}

} // namespace

// ==== IRQ ====
extern "C"
void TMR_IR_RX_IRQ_HNDLR() {
    Sys::IrqPrologue();
    Sys::LockFromIRQ();
    uint32_t flags = IRRcvr::tmr_rx.GetIrqFlags();
    IRRcvr::tmr_rx.ClearAllIrqFlags();
    if(settings.rx_mode == CYBERBEAR_MODE) {
        if(TimHw::IsUpdateFlagSet(flags))
            IRRcvr::ProcessCyberBearSample(Gpio::IsHi(IR_RX_DATA_PIN));
    }
    else if(settings.rx_mode == STANDARD_MODE) {
        IRRcvr::ProcessIrqI(flags, IRRcvr::tmr_rx.GetChnl0Value());
    }
    Sys::UnlockFromIRQ();
    Sys::IrqEpilogue();
}
#endif
