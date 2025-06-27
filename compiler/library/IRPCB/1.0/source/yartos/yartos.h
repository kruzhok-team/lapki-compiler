#ifndef YARTOS_YARTOS_H_
#define YARTOS_YARTOS_H_

#include <inttypes.h>
#include <stddef.h>

#include "EvtMsgIDs.h"
#include "gd32e11x_kl.h"
#include "yartos_cfg.h"

// Priority constants
#define NOPRIO 0UL        // Ready list header priority
#define IDLEPRIO 1UL      // Idle priority.
#define LOWPRIO 2UL       // Lowest priority.
#define NORMALPRIO 128UL  // Normal priority.
#define HIGHPRIO 255UL    // Highest priority.

// Type of time interval: 16 or 32 bits
#if SYS_TIM_RESOLUTION == 32
using systime_t = uint32_t;
#elif SYS_TIM_RESOLUTION == 16
using systime_t = uint16_t;
#else
#error "Bad SysTimer Resolution"
#endif

#define TIME_IMMEDIATE ((systime_t)0)
#define TIME_INFINITE ((systime_t) - 1)
#define TIME_MAX_INTERVAL ((systime_t) - 2)
#define TIME_MAX_SYSTIME \
    ((systime_t) - 1)  // Maximum value of system time before it wraps.

#define TIME_S2I(secs) \
    ((systime_t)((uint64_t)(secs) * (uint64_t)SYS_TIM_FREQUENCY))
#define TIME_MS2I(msecs)                                              \
    ((systime_t)((((uint64_t)(msecs) * (uint64_t)SYS_TIM_FREQUENCY) + \
                  (uint64_t)999) /                                    \
                 (uint64_t)1000))
#define TIME_US2I(usecs)                                              \
    ((systime_t)((((uint64_t)(usecs) * (uint64_t)SYS_TIM_FREQUENCY) + \
                  (uint64_t)999999) /                                 \
                 (uint64_t)1000000))

#define TIME_I2S(interval)                                           \
    (uint32_t)(((uint64_t)(interval) + (uint64_t)SYS_TIM_FREQUENCY - \
                (uint64_t)1) /                                       \
               (uint64_t)SYS_TIM_FREQUENCY)
#define TIME_I2MS(interval)                                  \
    (uint32_t)((((uint64_t)(interval) * (uint64_t)1000) +    \
                (uint64_t)SYS_TIM_FREQUENCY - (uint64_t)1) / \
               (uint64_t)SYS_TIM_FREQUENCY)
#define TIME_I2US(interval)                                  \
    (uint32_t)((((uint64_t)(interval) * (uint64_t)1000000) + \
                (uint64_t)SYS_TIM_FREQUENCY - (uint64_t)1) / \
               (uint64_t)SYS_TIM_FREQUENCY)

struct Thread;
typedef Thread *ThdReference_t;
enum class ThdState {
    Ready,
    Current,
    WTStart,
    Suspended,
    Queued,
    OnSemaphore,
    Sleeping
};

// Generic threads bidirectional linked list header and element.
struct ThreadsQueue {
    Thread *next;  // Next in the list/queue
    Thread *prev;  // Previous in the queue
    inline void Init();
    inline bool IsEmpty();
    inline Thread *FifoRemove();
    inline void Insert(Thread *tp);
};

class Semaphore {
   private:
    ThreadsQueue queue;  // Queue of the threads sleeping on this semaphore
    int32_t cnt;

   public:
    void Init(int32_t ACnt);
    retv WaitTimeoutS(systime_t Timeout);
    retv WaitTimeout(systime_t Timeout);
    inline void FastWaitI() {
        cnt--;
    }  // Can be used when the counter is known to be positive
    inline void FastSignalI() {
        cnt++;
    }  // Can be used when the counter is known to be not negative
    void SignalI();
    void Signal();
    inline int32_t GetCntI() { return cnt; }
};

#if 1  // ==== Virtual timer ====
// Type of a Virtual Timer callback function.
typedef void (*vtfunc_t)(void *p);

class VirtualTimer {
   private:
    VirtualTimer *next;            // Next timer in the list
    VirtualTimer *prev;            // Previous timer in the list
    systime_t delta;               // Time delta before timeout
    vtfunc_t pCallback = nullptr;  // Timer callback function pointer
    void *ptr = nullptr;           // Timer callback function parameter
    void DoSetI(systime_t delay, vtfunc_t vtfunc, void *APtr);
    void DoResetI();
    friend void VtDoTickI();
    friend retv SchGoSleepTimeoutS(ThdState newstate, systime_t timeout);

   public:
    inline bool IsArmedX() { return (pCallback != nullptr); }
    inline void ResetI() {
        if (IsArmedX()) DoResetI();
    }
    void Reset();
    inline void SetI(systime_t delay, vtfunc_t acallback, void *param) {
        ResetI();
        DoSetI(delay, acallback, param);
    }
    void Set(systime_t delay, vtfunc_t ACallback, void *param);
};

// ==== Event Timer ====
// Example: EvtTimer_t TmrS(TIME_S2I(1), EvtId::EverySecond,
// EvtTimer_t::Type::Periodic);
class EvtTimer : private VirtualTimer {
   private:
    systime_t period;
    EvtId evt_id;
    void StartI();
    friend void TmrEvtCallback(void *p);

   public:
    enum class Type { OneShot, Periodic } tmr_type;
    EvtTimer(systime_t APeriod, EvtId AEvtId, Type AType)
        : period(APeriod), evt_id(AEvtId), tmr_type(AType) {}
    void StartOrRestart();
    void StartOrRestart(systime_t NewPeriod);
    void StartIfNotRunning();
    inline void Stop() { Reset(); }
    inline bool IsRunning() { return IsArmedX(); }
    void SetNewPeriod_ms(uint32_t NewPeriod) { period = TIME_MS2I(NewPeriod); }
    void SetNewPeriod_s(uint32_t NewPeriod) { period = TIME_S2I(NewPeriod); }
};
#endif

/*
 * Type of stack and memory alignment enforcement.
 * In this architecture the stack alignment is enforced to 64 bits,
 * 32 bits alignment is supported by hardware but deprecated by ARM,
 * the implementation choice is to not offer the option.
 */
typedef uint64_t stkalign_t;
typedef void (*tfunc_t)(void);  // Thread function
typedef void *regarm_t;         // Type of a generic ARM register

struct McuExtCtx_t {  // port_extctx
    regarm_t r0;
    regarm_t r1;
    regarm_t r2;
    regarm_t r3;
    regarm_t r12;
    regarm_t lr_thd;
    regarm_t pc;
    regarm_t xpsr;
#if __FPU_PRESENT
    regarm_t s0;
    regarm_t s1;
    regarm_t s2;
    regarm_t s3;
    regarm_t s4;
    regarm_t s5;
    regarm_t s6;
    regarm_t s7;
    regarm_t s8;
    regarm_t s9;
    regarm_t s10;
    regarm_t s11;
    regarm_t s12;
    regarm_t s13;
    regarm_t s14;
    regarm_t s15;
    regarm_t fpscr;
    regarm_t reserved;
#endif /* CORTEX_USE_FPU */
};

struct McuIntCtx {
#if __FPU_PRESENT
    regarm_t s16;
    regarm_t s17;
    regarm_t s18;
    regarm_t s19;
    regarm_t s20;
    regarm_t s21;
    regarm_t s22;
    regarm_t s23;
    regarm_t s24;
    regarm_t s25;
    regarm_t s26;
    regarm_t s27;
    regarm_t s28;
    regarm_t s29;
    regarm_t s30;
    regarm_t s31;
#endif /* CORTEX_USE_FPU */
    regarm_t r4;
    regarm_t r5;
    regarm_t r6;
    regarm_t r7;
    regarm_t r8;
    regarm_t r9;
    regarm_t r10;
    regarm_t r11;
    regarm_t lr;
};

// Structure representing a thread
struct Thread {
    ThreadsQueue queue;  // Threads queue header
    uint32_t prio;       // Thread priority
    McuIntCtx *IntCtx;   // Processor context
    stkalign_t *wabase;  // This pointer is used for stack overflow checks
    ThdState state;      // Current thread state
    // State-specific fields. Valid in the specified state or condition and are
    // thus volatile
    retv msg;  // message sent to the thread by the waking thread or interrupt
               // handler. The value is valid after exiting the SchWakeupS()
               // function.
    union {
        void *wtobjp;  // Pointer to a generic "wait" synchronization object.
                       // Valid when the thread is in one of the wait states.
        ThdReference_t *wttrp;  // Pointer to a generic thread reference object,
                                // valid when the thread is in Suspended state.
        Semaphore *pSem;  // valid when the thread is in OnSemaphore state.
    } u;
};

#define THD_IRQ_REQUIRED_STACK \
    64  // Per-thread stack overhead for interrupts servicing.
#define THD_REQUIRED_STACK_SZ \
    (sizeof(McuIntCtx) + sizeof(McuIntCtx) + (size_t)THD_IRQ_REQUIRED_STACK)
#define MEM_ALIGN_MASK(a) \
    ((size_t)(a) -        \
     1UL)  // Alignment mask constant. Alignment must be a power of two
#define MEM_ALIGN_PREV(p, a) ((size_t)(p) & ~MEM_ALIGN_MASK(a))
#define MEM_ALIGN_NEXT(p, a) \
    MEM_ALIGN_PREV((size_t)(p) + MEM_ALIGN_MASK(a), (a))
#define MEM_IS_ALIGNED(p, a) (((size_t)(p) & MEM_ALIGN_MASK(a)) == 0U)
#define STACK_ALIGN sizeof(stkalign_t)
#define THD_WORKSPACE_SZ(n)                                      \
    MEM_ALIGN_NEXT(sizeof(Thread) + THD_REQUIRED_STACK_SZ + (n), \
                   sizeof(stkalign_t))

// Static working area allocation: used to allocate a static thread working area
// aligned as both position and size.
#define THD_WORKSPACE(WsName, Sz) \
    stkalign_t WsName[THD_WORKSPACE_SZ(Sz) / sizeof(stkalign_t)]

namespace Sys {

void Init();

// Threads
Thread *CreateThd(void *Workspace, size_t Sz, uint32_t Prio, tfunc_t ThdFunc);
Thread *GetSelfThd();

// Scheduler
void RescheduleS();

// inited by zeros in .cpp
extern volatile uint32_t Time_;
extern volatile uint32_t lastCNT_;
// Time and sleep

static inline systime_t GetSysTimeX() {
    return SYS_TIM->CNT;  // Time_;
}

inline systime_t TimeElapsedSince(systime_t start) {
    return (systime_t)(GetSysTimeX() - start);
}
retv Sleep(systime_t Delay_st);
retv SleepS(systime_t Delay_st);
retv SleepMilliseconds(uint32_t Delay_ms);
retv SleepSeconds(uint32_t Delay_s);
void WakeI(ThdReference_t *pThdref, retv Msg);

// Lock/Unlock
void Lock();
void Unlock();
void LockFromIRQ();
void UnlockFromIRQ();

// Put these at start and end of IRQ handler
void IrqPrologue();
void IrqEpilogue();

// Debug tools
void DbgAssert(bool IsOk, const char *MsgIfNotOk);
void DbgCheckClassI();

// Sys::Lock
static inline systime_t GetSysTime() {
    Sys::Lock();
    uint32_t time = Time_ + SYS_TIM->CNT;
    Sys::Unlock();
    return time;
}
}  // namespace Sys

#endif /* YARTOS_YARTOS_H_ */
