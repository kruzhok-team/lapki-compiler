/*
 * Command.cpp
 *
 *  Created on: 13/01/2024г.
 *        Author: Kreyl
 */

#include "shell.h"
#include "ir.h"
#include "max98357.h"
#include "app.h"
#include "Settings.h"
#include "ws2812bTim.h"
#include "led.h"

typedef void (*ftVoidPShell)(Shell *pshell);
extern const char* kBuildTime;
__attribute__((unused))
extern Neopixels npx_leds;

extern void Reboot();

class ShellCmd {
public:
    const char *name, *help;
    ftVoidPShell dispatcher;
    ShellCmd(const char* aname, ftVoidPShell adispatcher, const char* ahelp) :
        name(aname), help(ahelp), dispatcher(adispatcher) {}
};

// Dispatchers
static void DoPing(Shell *pshell) { pshell->Ok(); }
static void DoVersion(Shell *pshell) { pshell->Print("%S %S\r", APP_NAME, kBuildTime); }
static void DoReboot(Shell *pshell) { Reboot(); }

#if 1 // ====================== Testing related ================================
extern bool is_testing, beeped;
extern uint32_t tst_indx;
extern EvtTimer tmr_testing;

static const int32_t sinbuf[] = {
0, 1389, 2736, 3999, 5142, 6128, 6928, 7517, 7878, 8000,
7878, 7517, 6928, 6128, 5142, 4000, 2736, 1389, 0, -1389,
-2736, -4000, -5142, -6128, -6928, -7517, -7878, -8000, -7878, -7517,
-6928, -6128, -5142, -3999, -2736, -1389};
#define SIN_SZ    36

void TestIrRxCallbackI(uint8_t bit_cnt, uint16_t rcvd) { PrintfI("RX: 0x%X\r", rcvd); }
void cb_I2S_dma_doneI() {
    if(is_testing and tst_indx == 0) {
        Codec::TransmitBuf((void*)sinbuf, SIN_SZ);
    }
}

static void DoTest(Shell *pshell) {
    is_testing = true;
    beeped = false;
    IRRcvr::callbackI = TestIrRxCallbackI;
    // Gpios
    Gpio::SetupOut(Gpio1, Gpio::PushPull, Gpio::speed2MHz);
    Gpio::SetupOut(Gpio2, Gpio::PushPull, Gpio::speed2MHz);
    Gpio::SetupOut(Gpio3, Gpio::PushPull, Gpio::speed2MHz);
    Gpio::SetupOut(Gpio4, Gpio::PushPull, Gpio::speed2MHz);
    // Audio codec
    if(Codec::SetupSampleRate(48000) == retv::Ok) {
        Codec::cb_I2S_dma_doneI = cb_I2S_dma_doneI;
        Codec::TransmitBuf((void*)sinbuf, SIN_SZ);
    }
    else Printf("FS setup fail\r");
    tmr_testing.StartOrRestart();
}
#endif
#ifdef APP
static void GetSta(Shell *pshell) { pshell->Print("Hits: %d; Rnds: %d; mgzs: %d\r", hit_cnt, rounds_cnt, magazines_cnt); }

static void Restore(Shell *pshell) {
    Reset();
    pshell->Ok();
}

static void GetSettings(Shell *pshell) {
    for(auto pval : settings.values_arr) pval->PrintOnGet(pshell);
}

static void Set(Shell *pshell) {
    Cmd *pcmd = &pshell->cmd;
    const char* name;
    uint32_t v, N = 0;
    bool found;
    // Get pairs of values
    while((((name = pcmd->GetNextString()) != nullptr) and pcmd->GetNext(&v) == retv::Ok)) {
        found = false;
        for(ValueBase* const pval : settings.values_arr) {
            if(kl_strcasecmp(name, pval->name) == 0) {
                if(pval->CheckAndSetIfOk(v) == retv::Ok) {
                    pval->PrintOnNew(pshell);
                    N++;
                    found = true;
                    break;
                }
                else {
                    pval->PrintOnBad(pshell, v);
                    return;
                }
            } // if
        } // for
        if(!found) {
            pshell->Print("BadName: %S\r\n", name);
            break;
        }
    } // while
    pshell->Print("Set %u values\r\n", N);
    Reset();
}

static void SaveSettings(Shell *pshell) {
    if(settings.Save() == retv::Ok) pshell->Print("Saved\r\n");
    else pshell->Print("Saving fail\r\n");
}
static void LoadSettings(Shell *pshell) {
    settings.Load();
    Reset();
}

static void IrTx(Shell *pshell) {
    Cmd *pcmd = &pshell->cmd;
    int32_t pwr, bit_indx = 16; // Fill word starting from MSB
    uint16_t word = 0;
    if(pcmd->GetNext(&pwr).NotOk()) { pshell->BadParam(); return; }
    char *S;
    while((S = pcmd->GetNextString()) != nullptr and bit_indx > 0) {
        while(*S != '\0') {
            bit_indx--;
            if(*S++ != '0') word |= (1U << bit_indx); // !0 means 1
            if(bit_indx == 0) break; // it was the last bit
        }
    }
    uint32_t bit_cnt = 16 - bit_indx;
    if(bit_cnt == 0) { pshell->BadParam(); return; }
    Printf("Word16: 0x%X; bit number: %u\r", word, bit_cnt);
    IRLed::TransmitWord(word, bit_cnt, pwr, nullptr);
    pshell->Ok();
}

static void CmdFire(Shell *pshell) {
    FireSingleShot();
    pshell->Ok();
}

static void CmdBurst(Shell *pshell) {
    FireBurst();
    pshell->Ok();
}

static void CmdStop(Shell *pshell) {
    StopFire();
    pshell->Ok();
}
#endif
//static void CtrlSet(Shell *pshell) {
//    Cmd *pcmd = &pshell->cmd;
//    int32_t sta; // Fill word starting from MSB
//    if(pcmd->GetNext(&sta) != retv::Ok) { pshell->BadParam(); return; }
//    if(sta == 0) gpio::SetLo(Gpio3);
//    else gpio::SetHi(Gpio3);
//    pshell->Ok();
//}


#if 0 // ==== Debug ====
static void CtrlSet(Shell *pshell) {
    uint32_t In[2] = { 0, 0, };
    if(pshell->cmd.GetParams<uint32_t>(2, &In[0], &In[1]) == retv::Ok) {
        SetInputs(In);
        pshell->Ok();
    }
    else pshell->BadParam();
}

static void Npx(Shell *pshell) {
    Cmd *pcmd = &pshell->cmd;
    Color_t clr;
    if(pcmd->GetClrRGB(&clr) == retv::Ok) npx_leds.SetAll(clr);
//        uint32_t i=0;
//        while(pcmd->GetClrRGB(&clr) == retv::Ok) npx_leds.clr_buf[i++] = clr;
    npx_leds.SetCurrentColors();
}
#endif

//extern LedSmooth front_LEDs[FRONT_LEDS_CNT];
//extern LedSmooth side_LEDs[SIDE_LEDS_CNT];

//static void SetLed(Shell *pshell) {
//    Cmd *pcmd = &pshell->cmd;
//    uint32_t v;
//    if(pcmd->GetNext(&v) == retv::Ok) {
//        side_LEDs[0].Set(v);
//        pshell->Ok();
//    }
//    else pshell->BadParam();
//}

//static void Get(Shell *pshell) {
//    pshell->Print("0x%X %u %u\r", TIM0->INTF, TIM0->CH0CV, TIM0->CH1CV);
//}

// Commands
static const ShellCmd cmds[] = {
        {"Ping",    DoPing,    "Just to ask if anyone is there"},
        {"Version", DoVersion, "Show firmware version"},
        {"Test",    DoTest,    "Start hardware testing"},
        {"Reboot",  DoReboot,  "Reboot MCU"},
        // ==== App ====
#ifdef APP
        {"GetSta",  GetSta,    "Get current status of device (hit count etc.)"},
        {"Restore", Restore,   "Restore hits, rounds, magazines - everything"},
        {"GetSettings", GetSettings, "Get current values from Settings"},
        {"Set", Set, "Set one or more value of settings, ex: 'Set HitCnt 18, TeamID 7'. This will not save to Flash!"},
        {"SaveSettings", SaveSettings, "Save current settings to Flash"},
        {"LoadSettings", LoadSettings, "Load settings from Flash"},
        {"Fire", CmdFire, "Fire single shot"},
        {"Burst", CmdBurst, "Fire burst"},
        {"Stop", CmdStop, "Stop Fire"},
        // ==== Research ====
        {"IrTx", IrTx,"'IrTx Pwr, bits': transmit bits (up to 16) via IR LED at specified power. Divide bits into pieces for convenience. Ex: 'IRTx 90, 1100 0101 11 1001"},
#endif
        // ==== Debug ====
//        {"SetLed", SetLed, "set led PWM"},
//        {"Get", Get, "get"},
//        {"CtrlSet", CtrlSet,   "Set two control pins to specified value, ex: 'CtrlSet 1, 0'"},
//        {"NPX",     Npx,       "Set all NPX LEDs to specified RGB color, ex: 'Npx 18 255 180'"},
};

// Processing
void OnCmd(Shell *pshell) {
    Cmd *pcmd = &pshell->cmd;
    // Show help if needed
    if(pcmd->NameIs("Help")) {
        pshell->Print("==== Commands available ====\r\n");
        for(auto &scmd : cmds) pshell->Print("%S: %S\r\n", scmd.name, scmd.help);
    }
    else {
        for(auto &scmd : cmds) {
            if(pcmd->NameIs(scmd.name)) {
                scmd.dispatcher(pshell);
                return;
            }
        }
        //pshell->CmdUnknown();
    }
}
