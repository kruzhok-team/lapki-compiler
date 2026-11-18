#pragma once
#include <cstdint>
#define IR_TX_ENABLED 1
#ifndef IR_RX_ENABLED
#define IR_RX_ENABLED 0
#endif
#define IR_BIT_CNT_MAX 16
#define IR_LED 4
#define TMR_DAC_SMPL 6
#define TMR_IR_RX 1
#define TMR_IR_RX_IRQ 1
#define TMR_IR_RX_IRQ_HNDLR IRPCB_TEST_RX_IRQHandler
#ifndef IR_RX_DATA_PIN
#define IR_RX_DATA_PIN 1
#endif
#define IRQ_PRIO_HIGH 7
#define DAC_DMA 2
#define DMA_PRIO_HIGH 0
#define DMA_MEMSZ_8_BIT 0
#define DMA_PERSZ_8_BIT 0
#define DMA_MEM_INC 0
#define DMA_DIR_MEM2PER 0
#define DMA_TCIE 2
#define IRQ_PRIO_MEDIUM 0
using ftVoidVoid = void (*)();
static uint32_t primask;
static uint32_t __get_PRIMASK() { return primask; }
static void __disable_irq() { primask = 1; }
static void __set_PRIMASK(uint32_t mask) { primask = mask; }
namespace Sys {
inline uint32_t Time_ = 0;
inline uint32_t tick_ms = 0;
inline uint32_t GetSysTimeX() { return tick_ms; }
inline void IrqPrologue() {}
inline void IrqEpilogue() {}
inline void LockFromIRQ() {}
inline void UnlockFromIRQ() {}
}
namespace Gpio {
inline uint32_t input_levels = 0;
enum Pull { PullUp };
inline void SetupAnalog(int) {}
inline void SetupInput(int, Pull) {}
inline bool IsHi(int pin) { return input_levels & (1u << pin); }
}
namespace Nvic { inline void EnableVector(int, int) {} }
template<typename T> bool IsLike(T actual, T expected, uint32_t deviation) {
    return actual > expected ? actual - expected <= deviation : expected - actual <= deviation;
}
struct FakeRcu { void EnDAC() {} };
static FakeRcu rcu;
static FakeRcu *RCU = &rcu;
struct DAC_TypeDef {
    enum class Trigger { Tim6TRGO };
    uint8_t DAC0_R8DH = 0;
    void SetTrigger0(Trigger) {}
    void EnTrigger0() {}
    void EnDma0() {}
    void Enable0() {}
    void PutDataR0(uint32_t) {}
};
static DAC_TypeDef dac;
static DAC_TypeDef *DAC = &dac;
namespace RiseFall { constexpr int Rising = 1, Falling = 0; }
struct TimHw {
    enum class MasterMode { Update };
    enum class ChnlMode { CI1FE0 };
    enum class TriggerIn { CI1FE1 };
    enum class SlaveMode { Disable, Restart };
    mutable bool update_irq = false;
    mutable bool capture_irq = false;
    uint32_t flags = 0;
    uint32_t capture = 0;
    bool enabled = false;
    uint32_t frequency = 0;
    explicit TimHw(int) {}
    void Init() {}
    void SelectMasterMode(MasterMode) {}
    void SetUpdateFreqChangingTopValue(uint32_t value) { frequency = value; }
    void Disable() { enabled = false; }
    void Enable() { enabled = true; }
    void GenerateUpdateEvt() {}
    void DisableAutoreloadBuffering() {}
    void SetTopValue(uint32_t) {}
    void SetUpdateSrcOvfOnly() {}
    void SetInputFreqChangingPrescaler(uint32_t) {}
    void SetChnlMode(int, ChnlMode) {}
    void SetInputActiveEdge(int, int) {}
    void SetTriggerInput(TriggerIn) {}
    void SelectSlaveMode(SlaveMode) {}
    void EnChnl(int) {}
    void EnableIrqOnCompare0() { capture_irq = true; }
    void DisableIrqOnCompare0() { capture_irq = false; }
    void EnableIrqOnUpdate() { update_irq = true; }
    void DisableIrqOnUpdate() { update_irq = false; }
    void SetPrescaler(uint32_t) {}
    void SetCounter(uint32_t) {}
    void ClearAllIrqFlags() { flags = 0; }
    uint32_t GetIrqFlags() const { return flags; }
    uint32_t GetChnl0Value() const { return capture; }
    static bool IsUpdateFlagSet(uint32_t value) { return value & 1; }
};
struct DMA_t {
    mutable bool enabled = false;
    mutable bool completion_irq = false;
    mutable void *memory = nullptr;
    mutable uint16_t count = 0;
    void (*callback)(void *, uint32_t);
    DMA_t(int, void (*cb)(void *, uint32_t), void *, int) : callback(cb) {}
    void Init(volatile void *, uint32_t mode) const { SetMode(mode); }
    void Disable() const { enabled = false; }
    void DisableAndClearIRQ() const { enabled = false; completion_irq = false; }
    void SetMode(uint32_t mode) const { completion_irq = mode & DMA_TCIE; }
    void SetMemoryAddr(void *value) const { memory = value; }
    void SetTransferDataCnt(uint16_t value) const { count = value; }
    void Enable() const { enabled = true; }
};
namespace kIr {
constexpr uint32_t Header_us = 2400;
constexpr uint32_t Space_us = 600;
constexpr uint32_t Zero_us = 600;
constexpr uint32_t One_us = 1200;
constexpr uint32_t PauseAfter_us = 2400;
constexpr uint32_t InterBitTimeot_us = 900;
}

#if IR_RX_ENABLED
using ftVoidU8U16 = void (*)(uint8_t, uint16_t);
#include "CyberBearReceiver.h"
#endif
