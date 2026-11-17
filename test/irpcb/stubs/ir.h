#pragma once
#include <cstdint>
#define IR_TX_ENABLED 1
#define IR_RX_ENABLED 0
#define IR_BIT_CNT_MAX 16
#define IR_LED 4
#define TMR_DAC_SMPL 6
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
inline void LockFromIRQ() {}
inline void UnlockFromIRQ() {}
}
namespace Gpio { inline void SetupAnalog(int) {} }
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
struct TimHw {
    enum class MasterMode { Update };
    bool enabled = false;
    uint32_t frequency = 0;
    explicit TimHw(int) {}
    void Init() {}
    void SelectMasterMode(MasterMode) {}
    void SetUpdateFreqChangingTopValue(uint32_t value) { frequency = value; }
    void Disable() { enabled = false; }
    void Enable() { enabled = true; }
    void GenerateUpdateEvt() {}
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
}
