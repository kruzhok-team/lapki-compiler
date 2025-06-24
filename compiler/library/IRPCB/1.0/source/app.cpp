/*
 * app.cpp
 *
 *  Created on: 14.08.2023
 *      Author: Kreyl
 */

#include "app.h"

#include "Sequences.h"
#include "Settings.h"
#include "app_classes.h"
#include "beeper.h"
#include "gd_lib.h"
#include "gd_uart.h"
#include "ir.h"
#include "ir_pkt.h"
#include "led.h"
#include "shell.h"
#include "usb_msdcdc.h"

int32_t hit_cnt, rounds_cnt, magazines_cnt;
extern Beeper beeper;
extern bool is_testing;
extern CmdUart dbg_uart;
extern UsbMsdCdc usb_msd_cdc;
static bool burst_from_cmd = false;

AppMsgQueue evt_q_app;

#pragma region  // ============================== Controls
                // ===============================
// Outputs
CustomOutPin output_hits_present{Output_HitsPresent};

// Inputs
InputPinIrqTimed input_single_fire{Input_SingleFire};
InputPinIrqTimed input_burst_fire{Input_BurstFire};
PwmInputPin input_pwm{INPUT_PWM};

void InputPinIrqHandlerI() {
    uint32_t flags = EXTI->PD;
    // input for single fire
    if (flags & (1 << input_single_fire.pin_n)) {
        if (input_single_fire.CheckIfProcess())
            evt_q_app.SendNowOrExitI(AppEvt::StartFire);
    }
    // input for burst fire
    if (flags & (1 << input_burst_fire.pin_n)) {
        if (input_burst_fire.CheckIfProcess())
            evt_q_app.SendNowOrExitI(AppEvt::StartFire);
    }
}

// For Debug
static uint32_t iinputs[2];
void SetInputs(uint32_t ainputs[2]) {
    if (iinputs[0] == 0 and ainputs[0] == 1)
        evt_q_app.SendNowOrExit(AppEvt::StartFire);
    if (iinputs[1] == 0 and ainputs[1] == 1)
        evt_q_app.SendNowOrExit(AppEvt::StartFire);
    iinputs[0] = ainputs[0];
    iinputs[1] = ainputs[1];
}

bool DoBurstFire() {
    return (input_pwm.pwm_duty >= PWM_DUTY_BURST_FIRE_percent) or
           burst_from_cmd or input_burst_fire.IsHi() or iinputs[0] == 1;
}
#pragma endregion

#pragma region  // =========================== Indication
                // ================================
extern LedSmooth side_LEDs[SIDE_LEDS_CNT];
extern LedSmooth front_LEDs[FRONT_LEDS_CNT];

namespace Indication {

void PrintEverywhere(const char* format, ...) {
    va_list args;
    va_start(args, format);
    dbg_uart.IVsPrintf(format, args);
    va_start(args, format);
    usb_msd_cdc.IVsPrintf(format, args);
    va_end(args);
}

enum class IndiState { Idle, Reloading, MagazinesEnded, HitsEnded };
static IndiState state = IndiState::Idle;

void Shot() {
    beeper.StartOrRestart(bsqShot);
    for (auto& led : front_LEDs) led.StartOrRestart(lsqShot);
    PrintEverywhere("#Shot; %d/%d left\r\n", rounds_cnt, magazines_cnt);
}

void RoundsEnded() {
    Sys::Lock();
    // Beeper
    if (beeper.GetCurrentSequence() == bsqHit)
        beeper.SetNextSequenceI(bsqReloading);
    else
        beeper.StartOrRestartI(bsqReloading);
    // Leds
    if (side_LEDs[3].GetCurrentSequence() == lsqHit)
        side_LEDs[3].SetNextSequenceI(lsqReloading);
    else {
        side_LEDs[0].StopI();
        side_LEDs[1].StopI();
        side_LEDs[2].StopI();
        side_LEDs[3].StartOrRestartI(lsqReloading);
    }
    state = IndiState::Reloading;
    Sys::Unlock();
    PrintEverywhere("#RoundsEnded\r\n");
}

void MagazineReloaded() {
    Sys::Lock();
    // Beeper
    if (beeper.GetCurrentSequence() == bsqHit)
        beeper.SetNextSequenceI(bsqMagazReloaded);
    else
        beeper.StartOrRestartI(bsqMagazReloaded);
    // Leds
    if (side_LEDs[3].GetCurrentSequence() != lsqHit) {
        for (auto& Led : side_LEDs) Led.StopI();
    }
    state = IndiState::Idle;
    Sys::Unlock();
    PrintEverywhere("#MagazineReloaded\r\n");
}

void MagazinesEnded() {
    Sys::Lock();
    // Beeper
    if (beeper.GetCurrentSequence() == bsqHit)
        beeper.SetNextSequenceI(bsqMagazEnded);
    else
        beeper.StartOrRestartI(bsqMagazEnded);
    // Leds
    if (side_LEDs[3].GetCurrentSequence() == lsqHit)
        side_LEDs[3].SetNextSequenceI(lsqMagazinesEnded);
    else {
        side_LEDs[0].StopI();
        side_LEDs[1].StopI();
        side_LEDs[2].StopI();
        side_LEDs[3].StartOrRestartI(lsqMagazinesEnded);
    }
    state = IndiState::MagazinesEnded;
    Sys::Unlock();
    PrintEverywhere("#MagazinesEnded\r\n");
}

void Hit(uint32_t hit_from, int32_t damage) {
    Sys::Lock();
    //    OutPulseOnHit.PulseI(settings.pulse_len_hit_ms);
    beeper.StartOrRestartI(bsqHit);
    for (auto& Led : side_LEDs) Led.StartOrRestartI(lsqHit);
    switch (state) {
        case IndiState::Idle:
            beeper.SetNextSequenceI(nullptr);
            side_LEDs[3].SetNextSequenceI(nullptr);
            break;
        case IndiState::HitsEnded:
            beeper.SetNextSequenceI(bsqHitsEnded);
            side_LEDs[3].SetNextSequenceI(lsqHitsEnded);
            break;
        case IndiState::Reloading:
            beeper.SetNextSequenceI(bsqReloading);
            side_LEDs[3].SetNextSequenceI(lsqReloading);
            break;
        case IndiState::MagazinesEnded:
            beeper.SetNextSequenceI(bsqMagazEnded);
            side_LEDs[3].SetNextSequenceI(lsqMagazinesEnded);
            break;
    }
    Sys::Unlock();
    PrintEverywhere("#Hit from %d; damage %d; %d left\r\n", hit_from, damage,
                    hit_cnt);
}

void IrPktReceived() {  // Special case, when no hits indication required
    Sys::Lock();
    beeper.StartOrRestartI(bsqIrPktRcvd);
    for (auto& Led : side_LEDs) Led.StartOrRestartI(lsqHit);
    Sys::Unlock();
}

void HitsEnded() {
    Sys::Lock();
    // Beeper
    if (beeper.GetCurrentSequence() == bsqHit)
        beeper.SetNextSequenceI(bsqHitsEnded);
    else
        beeper.StartOrRestartI(bsqHitsEnded);
    output_hits_present.SetInactive();
    for (auto& Led : side_LEDs) Led.StartOrRestartI(lsqHitsEnded);
    Sys::Unlock();
    PrintEverywhere("#Hits Ended\r\n");
}

void HitsAdded(int32_t added_hits_number) {
    Sys::Lock();
    beeper.StartOrRestartI(bsqHitsAdded);
    for (auto& Led : side_LEDs) Led.StartOrRestartI(lsqHit);
    Sys::Unlock();
    PrintEverywhere("#Hits Added: %u; Hits cnt: %d\r\n", added_hits_number,
                    hit_cnt);
}

void RoundsAdded(int32_t magazines_to_add, int32_t rounds_left_to_add) {
    Sys::Lock();
    beeper.StartOrRestartI(bsqMagazReloaded);
    Sys::Unlock();
    PrintEverywhere("#Magazines Added: %u; magazines cnt: %d\r\n",
                    magazines_to_add, magazines_cnt);
    PrintEverywhere("#Rounds Added: %u; rounds cnt: %d\r\n", rounds_left_to_add,
                    rounds_cnt);
}

void Reset(bool quiet) {
    state = IndiState::Idle;
    for (auto& Led : front_LEDs) Led.Stop();
    for (auto& Led : side_LEDs) Led.Stop();
    if (!quiet) PrintEverywhere("#Reset\r\n");
}

}  // namespace Indication
#pragma endregion

#pragma region  // ========================= Reception processing
                // ========================
static systime_t prev_hit_time_st = 0;
static IRPkt last_rcvd_pkt;

void IrRxCallbackI(uint8_t bit_cnt, uint16_t rcvd) {
    evt_q_app.SendNowOrExitI(AppMsg(AppEvt::IrRx, bit_cnt, rcvd));
}

void ProcessRxPkt(IRPkt rx_pkt) {
    rx_pkt.Print((Shell*)&dbg_uart, "PktRx");
    rx_pkt.Print((Shell*)&usb_msd_cdc, "PktRx");
    static const int32_t kSuperDamage = 9999999L;

    // If retransmission is enabled, just retrtansmit & exit
    if (settings.transmit_what_rcvd.IsEnabled()) {
        last_rcvd_pkt = rx_pkt;
        Indication::IrPktReceived();
        return;
    }
    // Shot incoming
    if ((rx_pkt.bits_cnt == 14 or rx_pkt.bits_cnt == 16) and rx_pkt.zero == 0) {
        if (hit_cnt <= 0) return;  // Nothing to do when no hits left
        if (rx_pkt.player_id == *settings.player_id)
            return;  // Ignore pkt from self
        int32_t damage;
        // Special case: SuperDamageID - remove all hits if shot from him
        if (rx_pkt.player_id == *settings.super_damage_id) {
            damage = kSuperDamage;
            hit_cnt = 0;
        } else {
            // Ignore friendly fire (and pkt from self, too)
            if (rx_pkt.team_id == *settings.team_id) return;
            // Ignore if not enough time passed since last hit
            if (Sys::TimeElapsedSince(prev_hit_time_st) <
                TIME_S2I(*settings.min_delay_btw_hits_s))
                return;
            // Hit occured, decrement if not infinity
            damage = rx_pkt.GetDamageHits();
            if (!settings.hit_cnt.IsInfinity()) {
                hit_cnt = (damage < hit_cnt) ? (hit_cnt - damage) : 0;
            }
        }
        Indication::Hit(rx_pkt.player_id, damage);
        if (hit_cnt > 0)
            prev_hit_time_st = Sys::GetSysTimeX();
        else
            Indication::HitsEnded();
    }  // if 14/16 bit & zero
    else if (rx_pkt.bits_cnt == 16) {
        // Reset
        if (rx_pkt.word16 == static_cast<uint16_t>(PktType::NewGame)) {
            evt_q_app.SendNowOrExit(AppEvt::Reset);
        }
        // Custom pkt for applying SuperDamage on reception. Will never be equal
        // if rx_pkt_super_damage==-1 <=> disabled.
        else if (static_cast<int32_t>(rx_pkt.word16) ==
                 *settings.rx_pkt_super_damage) {
            hit_cnt = 0;
            Indication::Hit(rx_pkt.player_id, kSuperDamage);
            Indication::HitsEnded();
        }
        // Maybe, AddHealth or AddCartridges
        else {
            uint16_t pkt_type = rx_pkt.word16 & 0xFF00U;  // Zero MSB
            uint16_t pkt_value = rx_pkt.word16 & 0x00FF;  // Zero LSB
            if (pkt_type == static_cast<uint16_t>(PktType::AddHealth)) {
                Sys::Lock();
                hit_cnt = (pkt_value > *settings.hit_cnt) ? *settings.hit_cnt
                                                          : pkt_value;
                if (hit_cnt > 0)
                    output_hits_present.SetActive();
                else
                    output_hits_present.SetInactive();
                prev_hit_time_st = Sys::GetSysTimeX();
                Sys::Unlock();
                Indication::HitsAdded(pkt_value);
            } else if (pkt_type ==
                       static_cast<uint16_t>(PktType::AddCartridges)) {
                int32_t magazines_to_add =
                    pkt_value / *settings.rounds_in_magaz;
                int32_t rounds_left_to_add =
                    pkt_value - (*settings.rounds_in_magaz * magazines_to_add);
                Sys::Lock();
                magazines_cnt += magazines_to_add;
                if (magazines_cnt > *settings.magazines_cnt)
                    magazines_cnt = *settings.magazines_cnt;
                rounds_cnt += rounds_left_to_add;
                // Check if current magazine is overflown
                if (rounds_cnt > *settings.rounds_in_magaz)
                    rounds_cnt = *settings.rounds_in_magaz;
                prev_hit_time_st = Sys::GetSysTimeX();
                Sys::Unlock();
                Indication::RoundsAdded(magazines_to_add, rounds_left_to_add);
            }
        }
    }  // if bit_cnt == 16
}
#pragma endregion

#pragma region  // =============================== Firing
                // ================================
VirtualTimer fire_tmr;
IRPkt pkt_tx;
systime_t fire_start_time_st;
static bool is_firing;

// ==== Delay subsystem ====
void TmrCallback(void* p) {
    Sys::LockFromIRQ();
    evt_q_app.SendNowOrExitI((AppEvt)((uint32_t)p));
    Sys::UnlockFromIRQ();
    // Indication::PrintEverywhere("TmrCallback\n");
}

void StartDelay_s(int32_t delay_s, AppEvt aevt) {
    if (delay_s <= 0)
        evt_q_app.SendNowOrExit(aevt);  // Do it immediately
    else
        fire_tmr.Set(TIME_S2I(delay_s), TmrCallback, (void*)((uint32_t)aevt));
}

void StartDelay_ms(int32_t delay_ms, AppEvt aevt) {
    if (delay_ms <= 0)
        evt_q_app.SendNowOrExit(aevt);  // Do it immediately
    else
        fire_tmr.Set(TIME_MS2I(delay_ms), TmrCallback, (void*)((uint32_t)aevt));
}

void OnIrTxEndI() { evt_q_app.SendNowOrExitI(AppEvt::EndOfIrTx); }

void Fire() {
    is_firing = true;
    if (settings.transmit_what_rcvd.IsEnabled())
        pkt_tx = last_rcvd_pkt;
    else {
        if (!settings.rounds_in_magaz.IsInfinity()) rounds_cnt--;
        // Prepare pkt
        pkt_tx.bits_cnt = 16;  // All pkts excluding shot one
        pkt_tx.word16 = static_cast<uint16_t>(*settings.tx_pkt_type);
        switch (*settings.tx_pkt_type) {  // Modify pkt if needed
            case static_cast<uint16_t>(PktType::Shot):
                pkt_tx.word16 = 0;
                pkt_tx.player_id = *settings.player_id;
                pkt_tx.team_id = *settings.team_id;
                pkt_tx.damage_id = settings.tx_damage.bits;
                pkt_tx.bits_cnt = 14;
                break;

            case static_cast<uint16_t>(PktType::AddHealth):
            case static_cast<uint16_t>(PktType::AddCartridges):
                pkt_tx.word16 |=
                    static_cast<uint16_t>(*settings.tx_amount & 0xFFL);
                break;
            default:
                break;  // some special pkt type, do not modify
        }  // switch
    }

    pkt_tx.Print((Shell*)&dbg_uart, "PktTx");
    pkt_tx.Print((Shell*)&usb_msd_cdc, "PktTx");

    IRLed::TransmitWord(pkt_tx.word16, pkt_tx.bits_cnt, *settings.ir_tx_pwr,
                        OnIrTxEndI);
    fire_start_time_st = Sys::GetSysTimeX();
    Indication::Shot();
}
#pragma endregion

void Reset(bool quiet) {
    Sys::Lock();
    IRLed::ResetI();
    output_hits_present.SetMode(settings.pin_mode_gpio3.GetMode());
    output_hits_present.SetActive();
    //    OutPulseOnHit.ResetI();  // Removed to implement PWM input
    input_burst_fire.CleanIrqFlag();
    input_single_fire.CleanIrqFlag();
    is_firing = false;
    burst_from_cmd = false;
    rounds_cnt = *settings.rounds_in_magaz;
    magazines_cnt = *settings.magazines_cnt;
    fire_tmr.ResetI();
    hit_cnt = *settings.hit_cnt;
    prev_hit_time_st = Sys::GetSysTimeX();
    // IR tx
    IRLed::SetCarrierFreq(*settings.ir_tx_freq);
    Sys::Unlock();
    Indication::Reset(quiet);
}

// ==================================== Thread =================================
static THD_WORKSPACE(wa_app_thread, 256);
static void AppThread() {
    while (true) {
        AppMsg msg = evt_q_app.Fetch(TIME_INFINITE);
        if (is_testing) {  // Sleep if testing
            Sys::SleepMilliseconds(999);
            continue;
        }
        // Will be here when new Evt occur and if not in testing mode
        if (msg.evt == AppEvt::Reset)
            Reset();

        else if (msg.evt == AppEvt::IrRx)
            ProcessRxPkt(IRPkt(msg.data8, msg.data16));

        else if (hit_cnt > 0)
            switch (msg.evt) {  // Do nothing if no hits left
                case AppEvt::StartFire:
                    if (!is_firing and rounds_cnt > 0) Fire();
                    break;

                case AppEvt::NewPwmData:
                    //    Printf("pwm %u\r", input_pwm.pwm_duty);
                    if (input_pwm.pwm_duty > PWM_DUTY_SINGLE_FIRE_percent and
                        !is_firing and rounds_cnt > 0)
                        Fire();
                    break;

                case AppEvt::EndOfIrTx:  // Tx of several same pkts just ended
                    StartDelay_ms(*settings.shots_period_ms -
                                      TIME_I2MS(Sys::TimeElapsedSince(
                                          fire_start_time_st)),
                                  AppEvt::EndOfDelayBetweenShots);
                    break;

                case AppEvt::EndOfDelayBetweenShots:
                    is_firing = false;
                    if (rounds_cnt >
                        0) {  // Fire if needed and there are rounds left
                        if (DoBurstFire()) Fire();
                    } else {                      // no more rounds
                        if (magazines_cnt > 1) {  // Reload if possible (more
                                                  // than 0 magazines left)
                            if (!settings.magazines_cnt.IsInfinity())
                                magazines_cnt--;
                            Indication::RoundsEnded();
                            StartDelay_s(*settings.magaz_reload_delay_s,
                                         AppEvt::MagazineReloadDone);
                        } else {
                            magazines_cnt = 0;  // To avoid situation with 1
                                                // magaz with 0 rounds
                            Indication::MagazinesEnded();  // No magazines left
                        }
                    }
                    break;

                case AppEvt::MagazineReloadDone:
                    Indication::MagazineReloaded();
                    rounds_cnt = *settings.rounds_in_magaz;
                    if (DoBurstFire()) Fire();
                    break;

                default:
                    break;
            }  // switch Evt
    }  // while true
}

void AppInit() {
    /*
    evt_q_app.Init();
    // Control pins init
//    output_hits_ended.SetupOut(Gpio::OutMode::PushPull);
    output_hits_present.Init(); // Mode and Active will be set inside Reset()
//    OutPulseOnHit.Init(); // Removed to implement PWM input
    input_burst_fire.Init();
    input_single_fire.Init();
    */
    input_pwm.Init();
    return;
    Reset();
    // Create and start thread
    Sys::CreateThd(wa_app_thread, sizeof(wa_app_thread), NORMALPRIO, AppThread);
    // Start eternal fire if needed
    if (settings.fire_always.IsEnabled()) {
        burst_from_cmd = true;
        evt_q_app.SendNowOrExit(AppMsg(AppEvt::StartFire));
    }
}

void FireSingleShot() {
    burst_from_cmd = false;
    evt_q_app.SendNowOrExit(AppMsg(AppEvt::StartFire));
}

void FireBurst() {
    burst_from_cmd = true;
    evt_q_app.SendNowOrExit(AppMsg(AppEvt::StartFire));
}

void StopFire() { burst_from_cmd = false; }
