#include "ws2812bTim.h"
#include "shell.h"

#define NPX_DMA_MODE  DMA_PRIO_VERYHIGH | DMA_MEMSZ_16_BIT | DMA_PERSZ_16_BIT | DMA_MEM_INC | DMA_DIR_MEM2PER | DMA_TCIE

#if WS2812B_V2
#define ONE_DUR_ns      850UL
#define ZERO_DUR_ns     400UL

#define ONE_DUR_ticks   (ONE_DUR_ns / TICK_DUR_ns)
#define ZERO_DUR_ticks  (ZERO_DUR_ns / TICK_DUR_ns)
#define TIM_TOP_ticks   (ONE_DUR_ticks + ZERO_DUR_ticks)

#define RESET_BITS_CNT  240UL   // 300us / 1.260us per bit
#else

#endif

void NpxDmaDone(void *p, uint32_t flags) { ((Neopixels*)p)->OnDmaDone(); }

Neopixels::Neopixels(const NpxParams *apparams) :
    params(apparams), itim(apparams->ptim),
    dma_tx(apparams->dma_chnl_tx, NpxDmaDone, this) {}

void Neopixels::Init() {
    Gpio::SetupAlterFunc(params->pgpio, params->pin_n, Gpio::PushPull, Gpio::speed50MHz);
    itim.Init();
    itim.SetTopValue(TIM_TOP_ticks);
//    itim.SetInputFreqChangingPrescaler(2500000);
    // Setup output in PWM mode
    itim.EnPrimaryOutput();
    itim.SetChnlMode(params->tim_chnl, TimHw::ChnlMode::Output);
    itim.SetOutputCmpMode(params->tim_chnl, TimHw::CmpMode::PWM0HiLo);
    itim.EnableOutputShadow(params->tim_chnl);
    itim.EnChnl(params->tim_chnl);
    itim.EnableDmaOnUpdate();
    // Allocate memory
    clr_buf.resize(params->npx_cnt);
    ibitbuf_cnt = RESET_BITS_CNT + (params->npx_cnt * (uint32_t)params->clr_type);
    Printf("LedCnt: %u; BitBufCnt: %u\r", params->npx_cnt, ibitbuf_cnt);
    ibitbuf = (uint16_t*)malloc(ibitbuf_cnt * sizeof(uint16_t));
    for(uint32_t i=0; i<ibitbuf_cnt; i++) ibitbuf[i] = 0; // Zero it all, to zero head and tail
    // ==== DMA ====
    dma_tx.Init(itim.GetChnlRegAddr(params->tim_chnl), NPX_DMA_MODE);
    transmit_done = true;
}

__attribute__((always_inline))
static inline void PutBits(uint16_t **ptr, uint8_t byte) {
    uint16_t *p = *ptr;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    byte <<= 1UL;
    *p++ = (byte & 0x80)? ONE_DUR_ticks : ZERO_DUR_ticks;
    *ptr = p;
}

void Neopixels::SetCurrentColors() {
    transmit_done = false;
    // Fill bit buffer
    uint16_t *p = ibitbuf + (RESET_BITS_CNT / 2); // First and last bits are zero to form reset
    if(params->clr_type == NpxParams::ClrType::RGB) {
        for(auto &color : clr_buf) {
            PutBits(&p, color.G);
            PutBits(&p, color.R);
            PutBits(&p, color.B);
        }
    }
    else {
        for(auto &color : clr_buf) {
            PutBits(&p, color.G);
            PutBits(&p, color.R);
            PutBits(&p, color.B);
            PutBits(&p, color.W);
        }
    }
    // Start transmission
    itim.Disable();
    dma_tx.Disable();
    dma_tx.SetMemoryAddr(ibitbuf);
    dma_tx.SetTransferDataCnt(ibitbuf_cnt);
    dma_tx.Enable();
    itim.Enable();
}

void Neopixels::OnDmaDone() {
    itim.Disable();
    dma_tx.Disable();
    transmit_done = true;
    if(cb_on_transmit_end) cb_on_transmit_end();
}
