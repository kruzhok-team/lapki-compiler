#pragma once

/*
 * ========== WS2812 control module ==========
 * Only basic command "SetCurrentColors" is implemented, all other is up to
 * higher level software.
 * There are different timings for V2 and V5. Wir verachten sie.
 */
#define WS2812B_V2    TRUE
//#define WS2812B_V5    TRUE  // (and SK6812SIDE)

/*
 * WS2812 V2 requires timings, ns: (400 + 850) +- 150 each.
 * Reset must be:
 *    WS2812: 50uS => 125bit => ~16 bytes
 *    WS2813: 300uS => 750bit => 94 bytes
 *
 * WS2812 V5 requires timings, ns:
 * 0 is [220;380]+[580;1000]
 * 1 is [580;1000]+[580;1000]
 *
 * SK6812-SIDE (4020) requires timings, ns:
 * 0 is [200;400(typ 320)]+[800;...]
 * 1 is [620;1000(typ 640)]+[200;...]
 * Reset must be: 80uS => ~248bit => ~31 bytes
 *
== EXAMPLE ==
#define NPX_LED_CNT     (14*3)
#define NPX_PARAMS      PA0, TIM4, 0
#define NPX_PWR_EN      GPIOC, 0
// Npx LEDs
#define NPX_DMA             DMA1_Channel1 // Tim4 Update

static const NpxParams NParams{NPX_PARAMS, NPX_DMA, NPX_LED_CNT, NpxParams::ClrType::RGB};
Neopixels Leds{&NpxParams};
*/

#include "gd_lib.h"
#include "color.h"
#include "board.h"
#include <vector>

#define TICK_DUR_ns     20UL    // For 50MHz. Tune this.

typedef std::vector<Color_t> ColorBuf;

struct NpxParams {
    enum class ClrType {RGB=24UL, RGBW=32UL}; // RGB is 3 bytes = 24bits, RGBW is 4bytes = 32bits
    GPIO_TypeDef *pgpio;
    uint16_t pin_n;
    TIM_TypeDef *ptim;
    uint32_t tim_chnl;
    DMAChannel_t *dma_chnl_tx;
    uint32_t npx_cnt;
    ClrType clr_type;
    NpxParams(
            GPIO_TypeDef *apGpio, uint32_t apin,
            TIM_TypeDef *aptim, uint32_t atim_chnl,
            DMAChannel_t *adma_chnl_tx,
            uint32_t npx_cnt, ClrType AType) :
                pgpio(apGpio), pin_n(apin), ptim(aptim), tim_chnl(atim_chnl),
                dma_chnl_tx(adma_chnl_tx), npx_cnt(npx_cnt), clr_type(AType) {}
};


class Neopixels {
private:
    const NpxParams *params;
    uint32_t ibitbuf_cnt = 0;
    uint16_t *ibitbuf = nullptr;
    TimHw itim;
    DMA_t dma_tx;
    void OnDmaDone();
    friend void NpxDmaDone(void *p, uint32_t flags);
public:
    bool transmit_done = false;
    ftVoidVoid cb_on_transmit_end = nullptr;
    Neopixels(const NpxParams *apparams);
    void SetCurrentColors();
    ColorBuf clr_buf;
    void Init();
    void SetAll(Color_t clr) { std::fill(clr_buf.begin(), clr_buf.end(), clr); }
    bool AreOff() {
        for(auto &iclr : clr_buf) if(iclr != clBlack) return false;
        return true;
    }
};
