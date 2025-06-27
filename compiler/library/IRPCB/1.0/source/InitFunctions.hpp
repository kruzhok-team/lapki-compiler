#ifndef InitFunctions_HPP
#define InitFunctions_HPP

extern "C++" {
#include <string>

#include "IRReciever.h"
#include "MsgQ.h"
#include "Sequences.h"
#include "Serial.h"
#include "Settings.h"
#include "SpiFlash.h"
#include "app.h"
#include "beeper.h"
#include "gd_lib.h"
#include "gd_uart.h"
#include "ir.h"
#include "ir_pkt.h"
#include "kl_fs_utils.h"
#include "led.h"
#include "max98357.h"
#include "mem_msd_glue.h"
#include "usb_msdcdc.h"
#include "ws2812bTim.h"
#include "yartos.h"

#if 1  // ======================== Variables and defines
       // ========================
// Forever
extern const char *kBuildTime, *kBuildCfgName;
EvtMsgQ<EvtMsg, MAIN_EVT_Q_LEN> evt_q_main;
EvtMsg msg;
static const UartParams kCmdUartParams(115200, CMD_UART_PARAMS);
CmdUart dbg_uart{kCmdUartParams};
extern void OnCmd(Shell *pshell);  // See Command.cpp

LedBlinker sys_LED{LUMOS_PIN};
LedSmooth side_LEDs[SIDE_LEDS_CNT] = {
    {LED_PWM1}, {LED_PWM2}, {LED_PWM3}, {LED_PWM4}};
LedSmooth front_LEDs[FRONT_LEDS_CNT] = {{LED_FRONT1}, {LED_FRONT2}};

static const NpxParams kNpxParams{NPX_PARAMS, NPX_DMA, 17,
                                  NpxParams::ClrType::RGB};
Neopixels npx_leds{&kNpxParams};

Beeper beeper{BEEPER_PIN};

SpiFlash spi_flash(SPI0);
FATFS flash_fs;

EvtTimer tmr_uart_check(TIME_MS2I(UART_RX_POLL_MS), EvtId::UartCheckTime,
                        EvtTimer::Type::Periodic);
EvtTimer tmr_testing(TIME_MS2I(540), EvtId::TestingTime,
                     EvtTimer::Type::Periodic);

// Testing variables
#define TESTING_NPX_BRT 72
bool is_testing = false;
uint32_t tst_indx = 0;
bool beeped = false;
#endif

// Use watchdog to reset
void Reboot() {
    __disable_irq();
    while (true);
}

static inline void InitClk() {
    // Initial initialization
    FMC->EnableCashAndPrefetch();
    RCU->EnPwrMgmtUnit();
    FMC->SetLatency(50);  // 50 MHz required for NPX LEDs
    // Init Crystal
    retv state_xtal = RCU->EnableXTAL();
    if (state_xtal == retv::Ok) {
        RCU->SetPllPresel_XTAL();
        RCU->SetPrediv0Sel_XTALorIRC48M();  // XTAL is a source
        // 12MHz div 6 = 2MHz; 2MHz *25 = 50MHz. PLL input freq must be in [1;
        // 25] MHz, 8MHz typical
        RCU->SetPrediv0(6);
        RCU->SetPllSel_Prediv0();
        RCU->SetPllMulti(PllMulti::mul25);
        // Switch clk
        if (RCU->EnablePll() == retv::Ok) {
            RCU->SetAhbPrescaler(AhbPsc::div1);
            RCU->SwitchCkSys2PLL();
        }
    }
    // Setup IRC48M as usb clk and enable CTC trimmer
    if (RCU->EnableIRC48M() == retv::Ok) {
        RCU->SetCK48MSel_CKIRC48M();  // Use IRC48M as clk src instead of PLL
        RCU->EnCTC();                 // Enable trimmer
        // Setup trimmer: SOF freq is 1 kHz, rising front, no prescaler, input
        // is UsbSOF
        CTC->Setup(CTC_REFPOL_RISING, CTC_REFSEL_USBSOF, CTC_REFPSC_DIV_1,
                   1000);
        // If XTAL failed, use IRC as system clock source
        if (state_xtal != retv::Ok) {
            RCU->SetPllPresel_CKIRC48M();       // IRC48M is a source
            RCU->SetPrediv0Sel_XTALorIRC48M();  // IRC48M is a source
            // 48MHz div 12 = 4MHz; 4MHz *25 = 100MHz; 100MHz / 2 = 50MHz. PLL
            // input freq must be in [1; 25] MHz, 8MHz typical
            RCU->SetPrediv0(12);
            RCU->SetPllSel_Prediv0();
            RCU->SetPllMulti(PllMulti::mul25);
            // Switch clk
            if (RCU->EnablePll() == retv::Ok) {
                RCU->SetAhbPrescaler(AhbPsc::div2);
                RCU->SwitchCkSys2PLL();
            }
        }
    }
    Clk::UpdateFreqValues();
}

// works with evt_q_main instead of evt_q_app
void IrRxCallbackIMain(uint8_t bit_cnt, uint16_t rcvd) {
    evt_q_main.SendNowOrExitI(EvtMsg(EvtId::IrRx, bit_cnt, rcvd));
}

void watcher() {
    msg = evt_q_main.Fetch(TIME_INFINITE);
    switch (msg.id) {
        case EvtId::UartCheckTime:
            Watchdog::Reload();
            while (dbg_uart.TryParseRxBuff() == retv::Ok)
                OnCmd((Shell *)&dbg_uart);
            break;

        case EvtId::UsbCdcDataRcvd:
            while (usb_msd_cdc.TryParseRxBuff() == retv::Ok) {
                OnCmd((Shell *)&usb_msd_cdc);
                Serial::setMessage(((Shell *)&usb_msd_cdc)->cmd.name);
            }
            break;

        case EvtId::TestingTime:
            if (is_testing) {
                // Npx
                switch (tst_indx) {
                    case 0:
                        npx_leds.SetAll({TESTING_NPX_BRT, 0, 0});
                        break;
                    case 1:
                        npx_leds.SetAll({0, TESTING_NPX_BRT, 0});
                        break;
                    case 2:
                        npx_leds.SetAll({0, 0, TESTING_NPX_BRT});
                        break;
                    case 3:
                        npx_leds.SetAll({TESTING_NPX_BRT, TESTING_NPX_BRT,
                                         TESTING_NPX_BRT});
                        break;
                    default:
                        npx_leds.SetAll(clCyan);
                        break;
                }
                npx_leds.SetCurrentColors();
                // Send IR pkt
                IRLed::TransmitWord(0xCA11, 16, 180, nullptr);
                // Side Leds
                side_LEDs[tst_indx].StartOrRestart(lsqFadeInOut);
                // Front LEDs
                front_LEDs[tst_indx & 0x01].StartOrRestart(lsqFadeInOut);
                // Gpios
                Gpio::Set(Gpio1, tst_indx == 0);
                Gpio::Set(Gpio2, tst_indx == 1);
                Gpio::Set(Gpio3, tst_indx == 2);
                Gpio::Set(Gpio4, tst_indx == 3);
                // Buzzer
                if (tst_indx == 0 and !beeped) {
                    beeper.StartOrRestart(bsqBeepBeep);
                    beeped = true;
                }
                // Increment TstIndx
                if (tst_indx < 3)
                    tst_indx++;
                else
                    tst_indx = 0;
            }
            break;

        case EvtId::UsbReady:
            Printf("Usb ready\r");
            CTC->Enable();  // Start autotrimming of IRC48M
            break;
        case EvtId::IrRx:
            // TODO EvtMsg::value — 32битный тип, возможно следует переделать на
            // другую очередь событий или другое сообщение
            //  или вовсе создать свой поток, как это сделано в appThread
            IRReciever::update(msg.value_id, msg.value & 0xFFFF);
        default:
            break;
    }  // switch
}

void initAll() {
#ifndef BUILD_CFG_DEBUG  // Defined in makefile
    Watchdog::InitAndStart(999);
#endif
    InitClk();
    // ==== Disable JTAG ====
    RCU->EnAFIO();
    AFIO->DisableJtagDP();  // Disable JTAG, leaving SWD. Otherwise PB3 & PB4
                            // are occupied by JTDO & JTRST

    // ==== UART, RTOS & Event queue ====
    dbg_uart.Init();
    Sys::Init();

    evt_q_main.Init();
    Printf("\r%S %S\r\n", APP_NAME, kBuildTime);
    Clk::PrintFreqs();

    // ==== LEDs ====
    sys_LED.Init();
    sys_LED.StartOrRestart(lbsqStart);
    for (auto &led : side_LEDs) led.Init();
    for (auto &led : front_LEDs) led.Init();
    npx_leds.Init();
    npx_leds.SetAll(clBlack);
    npx_leds.SetCurrentColors();

    // ==== Audio ====
    Codec::Init();
    beeper.Init();

    // ==== Spi Flash, MsdGlue, filesystem ====
    AFIO->RemapSPI0_PB345();
    spi_flash.Init();
    spi_flash.Reset();
    SpiFlash::MemParams mp = spi_flash.GetParams();
    MsdMem::block_cnt = mp.sector_cnt;
    MsdMem::block_sz = mp.sector_sz;
    Printf("Flash: %u sectors of %u bytes\r", mp.sector_cnt, mp.sector_sz);
    if (mp.sector_cnt == 0 or mp.sector_sz == 0) Reboot();
    // Init filesystem
    if (f_mount(&flash_fs, "", 0) != FR_OK) Printf("FS error\r\n");

    // ==== USB ====
    usb_msd_cdc.Init();
    usb_msd_cdc.Connect();

    // ==== IR ==== TX LED SetFreq is called within App Reset, which is called
    // within AppInit
    IRLed::Init();
    IRRcvr::Init();
    IRRcvr::callbackI = IrRxCallbackIMain;

    // ==== App ====
    // settings.Load();
    // Printf("Pkt type: 0x%04X\r", settings.tx_pkt_type.v);
    // AppInit();

    // ==== Main evt cycle ====
    tmr_uart_check.StartOrRestart();
}
}

#endif  // InitFunctions_HPP