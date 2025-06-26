/*
 * yartos.cpp
 *
 *  Created on: 18 июл. 2023 г.
 *      Author: laurelindo
 */

#include "yartos.h"

#include "MsgQ.h"
#include "board.h"
#include "core_cm4.h"
#include "shell.h"

extern stkalign_t __main_thread_stack_base__;

// Asm function prototypes
extern "C" {
void _port_irq_epilogue();
void _port_switch(Thread *newtp, Thread *oldtp);
void _port_thread_start();
void _port_switch_from_isr();
void _port_exit_from_isr();
}

extern "C" void SysHalt(const char *reason);

// Hardware related
void InitSysHw();
void EnableRtos();
void PortLock();
void PortUnlock();
inline void SwitchContext(Thread *newtp, Thread *oldtp);

// Sys timer related
namespace SysTimer {
inline void Init();
inline void StopAlarm();
inline void StartAlarm(systime_t time);
inline void SetAlarm(systime_t time);
}  // namespace SysTimer

// ==== Priorities ====
#define CORTEX_PRIORITY_LEVELS (1U << CORTEX_PRIORITY_BITS)
#define CORTEX_MINIMUM_PRIORITY (CORTEX_PRIORITY_LEVELS - 1)
#define CORTEX_MAXIMUM_PRIORITY \
    0U  // The maximum allowed priority level is always zero.
#define CORTEX_PRIORITY_SVCALL \
    (CORTEX_MAXIMUM_PRIORITY + \
     1U)  // CORTEX_MAXIMUM_PRIORITY+1 reserves the CORTEX_MAXIMUM_PRIORITY
          // priority level as fast interrupts priority level
#define CORTEX_PRIORITY_PENDSV \
    (CORTEX_PRIORITY_SVCALL +  \
     1U)  // highest priority that cannot preempt the kernel
#define CORTEX_BASEPRI_DISABLED 0U  // Disabled value for BASEPRI register

// Priority level to priority mask conversion macro
#define CORTEX_PRIO_MASK(n) ((n) << (8U - (unsigned)CORTEX_PRIORITY_BITS))

#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE)
// Maximum usable priority for normal ISRs
#define CORTEX_MAX_KERNEL_PRIORITY (CORTEX_PRIORITY_SVCALL + 1U)
// BASEPRI level within kernel lock
#define CORTEX_BASEPRI_KERNEL CORTEX_PRIO_MASK(CORTEX_MAX_KERNEL_PRIORITY)
#else
#define CORTEX_MAX_KERNEL_PRIORITY 0U
#endif

#if 1  // =========================== Debug
       // =====================================
class SysDebug_t {
   public:
    const char *volatile panic_msg;
    int32_t isr_cnt;
    int32_t lock_cnt;

#if SYS_DBG_CHECK
    void Check(bool IsOk, const char *reason) {
        if (!IsOk) SysHalt(reason);
    }

    // Verifies that the system is in an appropriate state for invoking an
    // I-class API function
    void CheckClassI() {
        if ((isr_cnt < 0L) or (lock_cnt <= 0L)) SysHalt("#BadStateI");
    }
    void CheckClassS() {
        if ((isr_cnt != 0L) or (lock_cnt <= 0L)) SysHalt("#BadStateS");
    }
    void CheckLock() {
        if ((isr_cnt != 0L) or (lock_cnt != 0L)) SysHalt("#LockFail");
        lock_cnt = 1L;
    }
    void CheckLockFromIRQ() {
        if ((isr_cnt <= 0L) or (lock_cnt != 0L)) SysHalt("#LockIRQFail");
        lock_cnt = 1L;
    }
    void CheckUnlock() {
        if ((isr_cnt != 0L) or (lock_cnt <= 0L)) SysHalt("#UnlockFail");
        lock_cnt = 0L;
    }
    void CheckUnlockFromIRQ() {
        if ((isr_cnt <= 0L) or (lock_cnt <= 0L)) SysHalt("#UnlockIRQFail");
        lock_cnt = 0L;
    }
    void CheckEnterIRQ() {
        PortLock();
        if ((isr_cnt < 0L) or (lock_cnt != 0L)) SysHalt("#EnterIRQFail");
        isr_cnt++;
        PortUnlock();
    }
    void CheckLeaveIRQ() {
        PortLock();
        if ((isr_cnt <= 0L) or (lock_cnt != 0L)) SysHalt("#LeaveIRQFail");
        isr_cnt--;
        PortUnlock();
    }
#else
    void Check(bool IsOk, const char *reason) {}
    void CheckClassI() {}
    void CheckClassS() {}
    void CheckLock() {}
    void CheckLockFromIRQ() {}
    void CheckUnlock() {}
    void CheckUnlockFromIRQ() {}
    void CheckEnterIRQ() {}
    void CheckLeaveIRQ() {}
#endif

} dbg;

extern "C" void SysHalt(const char *reason) {
    __disable_irq();  // Disable system
    dbg.panic_msg = reason;
    PrintfNow(reason);
    while (true);  // Harmless infinite loop
}

// Called from asm
extern "C" {
void _dbg_check_lock() { dbg.CheckLock(); }
void _dbg_check_unlock() { dbg.CheckUnlock(); }
}
#endif

#if 1  // ========================= Virtual timers
       // ==============================
static inline systime_t TimeDiff(systime_t start, systime_t end) {
    return (systime_t)(end - start);
}

static inline systime_t TimeAdd(systime_t systime, systime_t interval) {
    return systime + (systime_t)interval;
}

/**
 * @brief   Virtual timers list header.
 * @note    The timers list is implemented as a double link bidirectional list
 *          in order to make the unlink time constant, the reset of a virtual
 *          timer is often used in the code.
 */
static struct VirtualTimersList_t {
    VirtualTimer *next; /**< @brief Next timer in the delta list. */
    VirtualTimer *prev; /**< @brief Last timer in the delta list. */
    systime_t delta = -1;
    systime_t lasttime;  // System time of the last tick event.
} vtlist;

// Virtual timers ticker
void VtDoTickI() {
    systime_t now;
    systime_t delta, nowdelta;
    // Loop through timers
    VirtualTimer *vtp = vtlist.next;
    while (true) {
        // Get the system time as a reference
        now = Sys::GetSysTimeX();
        nowdelta = TimeDiff(vtlist.lasttime, now);
        /* The list scan is limited by the timers header having
          "vtlist.vt_delta == (systime_t)-1" which is greater than all deltas */
        if (nowdelta < vtp->delta) break;
        // Process all timers between "vtp->lasttime" and now
        do {
            // The "last time" becomes this timer's expiration time
            vtlist.lasttime += vtp->delta;
            nowdelta -= vtp->delta;

            vtp->next->prev = (VirtualTimer *)&vtlist;
            vtlist.next = vtp->next;
            vtfunc_t pCallback = vtp->pCallback;
            vtp->pCallback = nullptr;

            // When the list is empty, the Sys Timer alarm must be stopped
            if (vtlist.next == (VirtualTimer *)&vtlist) SysTimer::StopAlarm();

            // The callback is invoked outside the kernel critical zone
            Sys::UnlockFromIRQ();
            pCallback(vtp->ptr);
            Sys::LockFromIRQ();

            vtp = vtlist.next;  // Get next element in the list
        } while (vtp->delta <= nowdelta);
    }  // while(true)

    // If the list is empty, there is nothing else to do
    if (vtlist.next == (VirtualTimer *)&vtlist) return;

    // The "unprocessed nowdelta" time slice is added to "last time" and
    // subtracted to next timer's delta
    vtlist.lasttime += nowdelta;
    vtlist.next->delta -= nowdelta;

    // Recalculate the next alarm time
    delta = vtp->delta - TimeDiff(vtlist.lasttime, now);
    if (delta < (systime_t)SYS_ST_TIMEDELTA)
        delta = (systime_t)SYS_ST_TIMEDELTA;

    SysTimer::SetAlarm(TimeAdd(now, delta));
}

void VirtualTimer::DoSetI(systime_t delay, vtfunc_t vtfunc, void *APtr) {
    dbg.CheckClassI();
    dbg.Check((vtfunc != NULL) && (delay != TIME_IMMEDIATE), "Bad vt params");
    ptr = APtr;
    pCallback = vtfunc;
    systime_t now = Sys::GetSysTimeX();
    // If the delay is lower than the minimum safe delta then it is raised to
    // the minimum safe value
    if (delay < (systime_t)SYS_ST_TIMEDELTA)
        delay = (systime_t)SYS_ST_TIMEDELTA;
    // Special case when the timers list is empty
    if (&vtlist == (VirtualTimersList_t *)vtlist.next) {
        // The delta list is empty => current time becomes the new delta list
        // base time, the timer is inserted
        vtlist.lasttime = now;
        vtlist.next = this;
        vtlist.prev = this;
        next = (VirtualTimer *)&vtlist;
        prev = (VirtualTimer *)&vtlist;
        delta = delay;
        // Start the SysTimer alarm as this is the first timer in the list
        SysTimer::StartAlarm(TimeAdd(vtlist.lasttime, delay));
        return;
    }
    VirtualTimer *p = vtlist.next;  // Pointer to the first element in the delta
                                    // list, which is non-empty
    // Delay as delta from 'lasttime'. Note, it can overflow and the value
    // becomes lower than 'now'
    systime_t Fdelta = TimeDiff(vtlist.lasttime, now) + delay;
    if (Fdelta < TimeDiff(vtlist.lasttime, now)) {
        /* Scenario where a very large delay excedeed the numeric range, it
           requires a special handling. We need to skip the first element and
           adjust the delta to wrap back in the previous numeric range.*/
        Fdelta -= p->delta;
        p = p->next;
    } else if (Fdelta < p->delta) {
        systime_t deadline_delta;
        // A small delay that will become the first element in the delta list
        // and next deadline
        deadline_delta = Fdelta;
        SysTimer::SetAlarm(TimeAdd(vtlist.lasttime, deadline_delta));
    }
    // The delta list is scanned in order to find the correct position for this
    // timer
    while (p->delta < Fdelta) {
        // Debug assert if the timer is already in the list
        dbg.Check(p != this, "timer already armed");
        Fdelta -= p->delta;
        p = p->next;
    }
    // Insert the timer to the delta list
    next = p;
    prev = next->prev;
    prev->next = this;
    p->prev = this;
    delta = Fdelta;
    // Calculate new delta for the following entry
    p->delta -= Fdelta;
    // Special case when the timer is in last position in the list, the value in
    // the header must be restored
    vtlist.delta = (systime_t)-1;
}

void VirtualTimer::DoResetI() {
    dbg.CheckClassI();
    dbg.Check(pCallback != NULL, "timer not set or already triggered");
    pCallback = nullptr;
    // If the timer is not the first of the list then it is simply unlinked,
    // else the operation is more complex
    if (vtlist.next != this) {
        // Remove the element from the delta list
        prev->next = next;
        next->prev = prev;
        // Add delta to the next element, if it is not the last one
        if (&vtlist != (VirtualTimersList_t *)next) next->delta += delta;
        return;
    }
    // Remove the first timer from the list
    vtlist.next = next;
    vtlist.next->prev = (VirtualTimer *)&vtlist;
    // If the list become empty then stop sys timer and return
    if (&vtlist == (VirtualTimersList_t *)vtlist.next) {
        SysTimer::StopAlarm();
        return;
    }
    // The delta of the removed timer is added to the new first timer
    vtlist.next->delta += delta;
    // Distance in ticks between the last alarm event and current time
    systime_t nowdelta = TimeDiff(vtlist.lasttime, Sys::GetSysTimeX());
    /* If the current time surpassed the time of the next element in list
     then the event interrupt is already pending, just return.*/
    if (nowdelta >= vtlist.next->delta) return;
    // Distance from the next scheduled event and now
    systime_t Fdelta = vtlist.next->delta - nowdelta;
    // Make sure to not schedule an event closer than SYS_ST_TIMEDELTA ticks
    // from now
    if (Fdelta < (systime_t)SYS_ST_TIMEDELTA)
        Fdelta = nowdelta + (systime_t)SYS_ST_TIMEDELTA;
    else
        Fdelta = nowdelta + Fdelta;
    SysTimer::SetAlarm(TimeAdd(vtlist.lasttime, delta));
}

void VirtualTimer::Reset() {
    Sys::Lock();
    ResetI();
    Sys::Unlock();
}

void VirtualTimer::Set(systime_t delay, vtfunc_t acallback, void *param) {
    Sys::Lock();
    SetI(delay, acallback, param);
    Sys::Unlock();
}

#if 1  // ==== Event Timer ====
void TmrEvtCallback(void *p) {
    Sys::LockFromIRQ();
    EvtTimer *vt = (EvtTimer *)p;
    evt_q_main.SendNowOrExitI(EvtMsg(vt->evt_id));
    if (vt->tmr_type == vt->Type::Periodic) vt->StartI();
    Sys::UnlockFromIRQ();
}

// Will be before start
void EvtTimer::StartI() { SetI(period, TmrEvtCallback, this); }

void EvtTimer::StartOrRestart() {
    Sys::Lock();
    StartI();
    Sys::Unlock();
}
void EvtTimer::StartOrRestart(systime_t NewPeriod) {
    Sys::Lock();
    period = NewPeriod;
    StartI();
    Sys::Unlock();
}
void EvtTimer::StartIfNotRunning() {
    Sys::Lock();
    if (!IsArmedX()) StartI();
    Sys::Unlock();
}
#endif  // EvtTimer
#endif  // Virtual Timers

#if 1  // ============================ Thread Queue
       // =============================
static struct ReadyList_t {
    ThreadsQueue queue;
    uint32_t prio;
    Thread *current;  // The currently running thread
} rlist;

// Current thread pointer
#define currp rlist.current

inline void ThreadsQueue::Insert(Thread *tp) {
    tp->queue.next = (Thread *)this;
    tp->queue.prev = prev;
    tp->queue.prev->queue.next = tp;
    prev = tp;
}

inline Thread *ThreadsQueue::FifoRemove() {
    Thread *tp = next;
    next = tp->queue.next;
    next->queue.prev = (Thread *)this;
    return tp;
}

static inline Thread *queue_dequeue(Thread *tp) {
    tp->queue.prev->queue.next = tp->queue.next;
    tp->queue.next->queue.prev = tp->queue.prev;
    return tp;
}

inline void ThreadsQueue::Init() {
    next = (Thread *)this;
    prev = (Thread *)this;
}

inline bool ThreadsQueue::IsEmpty() { return (next == (Thread *)this); }
#endif

#if 1  // ============= Scheduler ==============
// Returns the priority of the first thread on the given ready list.
#define firstprio(rlp) ((rlp)->next->prio)

// Evaluates if preemption is required
bool SchIsPreemptionRequired() {
    uint32_t p1 = firstprio(&rlist.queue);
    uint32_t p2 = currp->prio;
    return p1 > p2;
}

// Inserts a thread in the Ready List placing it behind all threads with higher
// or equal priority.
Thread *SchReadyI(Thread *tp) {
    dbg.CheckClassI();
    dbg.Check(tp != NULL, "#TP nullptr");
    dbg.Check(tp->state != ThdState::Ready, "invalid state");
    tp->state = ThdState::Ready;
    ;
    Thread *cp = (Thread *)&rlist.queue;
    do {
        cp = cp->queue.next;
    } while (cp->prio >= tp->prio);
    // Insertion on prev
    tp->queue.next = cp;
    tp->queue.prev = cp->queue.prev;
    tp->queue.prev->queue.next = tp;
    cp->queue.prev = tp;
    return tp;
}

// Inserts a thread in the Ready List placing it ahead all threads with higher
// or equal priority.
Thread *SchReadyAheadI(Thread *tp) {
    Thread *cp;
    dbg.CheckClassI();
    dbg.Check(tp != nullptr, "#TP nullptr");
    dbg.Check(tp->state != ThdState::Ready, "invalid state");
    tp->state = ThdState::Ready;
    cp = (Thread *)&rlist.queue;
    do {
        cp = cp->queue.next;
    } while (cp->prio > tp->prio);
    // Insert on prev
    tp->queue.next = cp;
    tp->queue.prev = cp->queue.prev;
    tp->queue.prev->queue.next = tp;
    cp->queue.prev = tp;
    return tp;
}

/**
 * @brief   Switches to the first thread on the runnable queue.
 * @details The current thread is positioned in the ready list ahead of all
 *          threads having the same priority.
 */
void SchDoRescheduleAhead() {
    Thread *oldtp = currp;
    // Pick the first thread from the ready queue and make it current
    currp = rlist.queue.FifoRemove();
    currp->state = ThdState::Current;
    // Place in ready list ahead of peers
    oldtp = SchReadyAheadI(oldtp);
    SwitchContext(currp, oldtp);  // Finally, switch context
}

extern "C" void SchDoRescheduleI() {
    Thread *oldtp = currp;
    // Pick the first thread from the ready queue and make it current
    currp = rlist.queue.FifoRemove();
    currp->state = ThdState::Current;
    // If the round-robin mechanism is disabled then the thread goes always
    // ahead of its peers
    oldtp = SchReadyAheadI(oldtp);
    SwitchContext(currp, oldtp);  // Finally, switch context
}

static inline bool SchIsReschRequiredI() {
    dbg.CheckClassI();
    return firstprio(&rlist.queue) > currp->prio;
}

// Puts the current thread to sleep into the specified state
void SchGoSleepS(ThdState newstate) {
    Thread *oldtp = currp;
    dbg.CheckClassS();
    oldtp->state = newstate;
    // Next thread in ready list becomes current
    currp = rlist.queue.FifoRemove();
    currp->state = ThdState::Current;
    SwitchContext(currp, oldtp);  // Finally, switch context
}

// Timeout wakeup callback
static void wakeup(void *p) {
    Thread *tp = (Thread *)p;
    Sys::LockFromIRQ();
    switch (tp->state) {
        case ThdState::Ready:  // Special case: thread has been made ready by
                               // another thread with higher priority
            Sys::UnlockFromIRQ();
            return;
        case ThdState::Suspended:
            *tp->u.wttrp = NULL;
            break;
        case ThdState::OnSemaphore:
            tp->u.pSem->FastSignalI();
            // Fall through
        case ThdState::Queued:  // States requiring dequeuing
            queue_dequeue(tp);
            break;
        default:  // Any other state, nothing to do
            break;
    }
    tp->msg = retv::Timeout;
    SchReadyI(tp);
    Sys::UnlockFromIRQ();
}

/* Put the current thread to sleep into the specified state with timeout.
 * If the thread is not awakened explicitly within the specified timeout then
 * it is forcibly awakened with a MSG_TIMEOUT low level message */
retv SchGoSleepTimeoutS(ThdState newstate, systime_t timeout) {
    dbg.CheckClassS();
    if (timeout == TIME_INFINITE)
        SchGoSleepS(newstate);  // Sleep forever
    else {
        VirtualTimer vt;
        vt.DoSetI(timeout, wakeup, currp);
        SchGoSleepS(newstate);
        // Will be here after awake
        if (vt.IsArmedX())
            vt.DoResetI();  // Stop timer if thread is woken by something else
    }
    return currp->msg;
}

/* The thread is inserted into the ready list or immediately made running
depending on its relative priority compared to the current thread. The thread
must not be already inserted in any list through its next and prev or list
corruption would occur. It is equivalent to a chSchReadyI() followed by a
RescheduleS() but much more efficient. */
void SchWakeupS(Thread *newtp, retv msg) {
    Thread *oldtp = currp;
    dbg.CheckClassS();
    dbg.Check((rlist.queue.next == (Thread *)&rlist.queue) or
                  (rlist.current->prio >= rlist.queue.next->prio),
              "priority order violation");
    // Store the message to be retrieved by the target thread when it will
    // restart execution
    newtp->msg = msg;
    /* If the waken thread has a not-greater priority than the current one then
       it is just inserted in the ready list, else it made running immediately
       and the invoking thread goes in the ready list instead.*/
    if (newtp->prio <= oldtp->prio)
        SchReadyI(newtp);
    else {
        oldtp = SchReadyAheadI(oldtp);
        currp = newtp;  // The extracted thread is marked as current
        newtp->state = ThdState::Current;
        SwitchContext(newtp, oldtp);  // Swap them finally
    }
}
#endif  // Scheduler

#if 1  // ============================= Semaphore
       // ===============================
void Semaphore::Init(int32_t ACnt) {
    queue.Init();
    cnt = ACnt;
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout
 *
 * @param[in] timeout   the number of ticks before the operation timeouts, the
 * following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 * released from the semaphore.
 * @retval Ok           if the thread has not stopped on the semaphore or the
 * semaphore has been signaled.
 * @retval Reset        if the semaphore has been reset using @p
 * Semaphore.Reset().
 * @retval Timeout      if the semaphore has not been signaled or reset within
 * the specified timeout.
 *
 * @sclass
 */
retv Semaphore::WaitTimeoutS(systime_t Timeout) {
    dbg.CheckClassS();
    dbg.Check((cnt >= 0 and queue.IsEmpty()) or (cnt < 0 and !queue.IsEmpty()),
              "inconsistent semaphore");
    if (--cnt < 0L) {
        if (Timeout == TIME_IMMEDIATE) {
            cnt++;
            return retv::Timeout;
        }
        currp->u.pSem = this;
        queue.Insert(currp);  // Insert the current thd to the semaphore queue
        return SchGoSleepTimeoutS(ThdState::OnSemaphore, Timeout);
    }
    return retv::Ok;
}

/**
 * @brief   Performs a wait operation on a semaphore with timeout
 *
 * @param[in] timeout   the number of ticks before the operation timeouts, the
 * following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A message specifying how the invoking thread has been
 * released from the semaphore.
 * @retval Ok           if the thread has not stopped on the semaphore or the
 * semaphore has been signaled.
 * @retval Reset        if the semaphore has been reset using @p
 * Semaphore.Reset().
 * @retval Timeout      if the semaphore has not been signaled or reset within
 * the specified timeout.
 *
 * @api
 */
retv Semaphore::WaitTimeout(systime_t Timeout) {
    retv msg;
    Sys::Lock();
    msg = WaitTimeoutS(Timeout);
    Sys::Unlock();
    return msg;
}

/**
 * @brief   Performs a signal operation on a semaphore.
 * @post    This function does not reschedule so a call to a rescheduling
 *          function must be performed before unlocking the kernel. Note that
 *          interrupt handlers always reschedule on exit so an explicit
 *          reschedule must not be performed in IRQs.
 *
 * @iclass
 */
void Semaphore::SignalI() {
    dbg.CheckClassI();
    dbg.Check((cnt >= 0 and queue.IsEmpty()) or (cnt < 0 and !queue.IsEmpty()),
              "inconsistent semaphore");
    if (++cnt <= 0) {
        Thread *tp = queue.FifoRemove();  // Remove curr thd from queue
        tp->msg = retv::Ok;
        SchReadyI(tp);  // ...and start it
    }
}

/**
 * @brief   Performs a signal operation on a semaphore.
 *
 */
void Semaphore::Signal() {
    Sys::Lock();
    dbg.Check((cnt >= 0 and queue.IsEmpty()) or (cnt < 0 and !queue.IsEmpty()),
              "inconsistent semaphore");
    if (++cnt <= 0) SchWakeupS(queue.FifoRemove(), retv::Ok);
    Sys::Unlock();
}

#endif  // Semaphore

// Setup the context switching frame represented by an McuIntCtx_t structure
#define SETUP_CONTEXT(pThd, pThdFunc)                                          \
    {                                                                          \
        (pThd)->IntCtx = (McuIntCtx *)((uint8_t *)(pThd) - sizeof(McuIntCtx)); \
        (pThd)->IntCtx->r4 = (regarm_t)(pThdFunc);                             \
        (pThd)->IntCtx->lr = (regarm_t)_port_thread_start;                     \
    }

#if 1  // ==== Idle Thread related ====
/* This thread has the lowest priority in the system, its role is only to
   serve interrupts in its context, using the lowest energy saving
   mode compatible with the system status.
   Stack size set to 16 because the idle thread does have a stack frame when
   compiling without optimizations. You may reduce this value to zero when
   compiling with optimizations. */
#define IDLE_THREAD_STACK_SIZE 16
THD_WORKSPACE(wsIdleThd, IDLE_THREAD_STACK_SIZE);
static void IdleThd() {
    while (true) __WFI();
}
#endif

static Thread mainthread;

namespace Sys {  // =========================== Sys
                 // =============================

volatile uint32_t Time_ = SYS_TIM->GetTopValue();
volatile uint32_t lastCNT_ = SYS_TIM->CNT;
volatile uint32_t sys_tim_overflows = 0;

void Init() {
    // Init Scheduler
    rlist.queue.Init();
    rlist.prio = NOPRIO;
    // Init VirtualTimers
    vtlist.next = (VirtualTimer *)&vtlist;
    vtlist.prev = (VirtualTimer *)&vtlist;
    vtlist.delta = (systime_t)-1;
    vtlist.lasttime = (systime_t)0;
    // Set current flow as main thread
    currp = &mainthread;
    currp->prio = NORMALPRIO;
    // Stack check init. Set up the base address of the static main thread
    // stack, the symbol must be provided externally
    currp->wabase = &__main_thread_stack_base__;
    currp->state = ThdState::Current;
    InitSysHw();
    EnableRtos();
    CreateThd(wsIdleThd, sizeof(wsIdleThd), IDLEPRIO, IdleThd);
}

Thread *CreateThd(void *Workspace, size_t Sz, uint32_t Prio, tfunc_t ThdFunc) {
    dbg.Check((Workspace != NULL) && MEM_IS_ALIGNED(Workspace, STACK_ALIGN) &&
                  (Sz >= THD_WORKSPACE_SZ(0)) &&
                  MEM_IS_ALIGNED(Sz, STACK_ALIGN) && (Prio <= HIGHPRIO),
              "Bad CreateThd params");
    Lock();
    /* The thread structure is laid out in the upper part of the thread
       workspace. The thread position structure is aligned to the required
       stack alignment because it represents the stack top.*/
    Thread *tp = (Thread *)((uint8_t *)Workspace + Sz -
                            MEM_ALIGN_NEXT(sizeof(Thread), STACK_ALIGN));
    tp->wabase = (stkalign_t *)Workspace;  // Stack boundary
    SETUP_CONTEXT(
        tp, ThdFunc);  // Setup the port-dependent part of the working area
    tp->prio = Prio;
    tp->state = ThdState::WTStart;
    // Start the thread immediately
    SchWakeupS(tp, retv::Ok);
    Unlock();
    return tp;
}

Thread *GetSelfThd() { return rlist.current; }

void RescheduleS() {
    dbg.CheckClassS();
    if (SchIsReschRequiredI()) SchDoRescheduleAhead();
}

#if 1  // ==== Time ====
retv SleepS(systime_t Delay_st) {
    return SchGoSleepTimeoutS(ThdState::Sleeping, Delay_st);
}

retv Sleep(systime_t Delay_st) {
    Lock();
    retv r = SleepS(Delay_st);
    Unlock();
    return r;
}

retv SleepMilliseconds(uint32_t Delay_ms) { return Sleep(TIME_MS2I(Delay_ms)); }
retv SleepSeconds(uint32_t Delay_s) { return Sleep(TIME_S2I(Delay_s)); }

void WakeI(ThdReference_t *pThdref, retv amsg) {
    dbg.CheckClassI();
    if (*pThdref != nullptr) {
        Thread *pThd = *pThdref;
        *pThdref = nullptr;
        if (pThd->state != ThdState::Ready) {
            pThd->msg = amsg;
            // Make it ready and insert in ready list
            pThd->state = ThdState::Ready;
            Thread *cp = (Thread *)&rlist.queue;
            do {
                cp = cp->queue.next;
            } while (cp->prio >= pThd->prio);
            // Insert on prev
            pThd->queue.next = cp;
            pThd->queue.prev = cp->queue.prev;
            pThd->queue.prev->queue.next = pThd;
            cp->queue.prev = pThd;
        }
    }
}
#endif

#if 1  // === Lock / Unlock ===
void Lock() {
    PortLock();
    dbg.CheckLock();
}

void Unlock() {
    dbg.CheckUnlock();
    PortUnlock();
}

void LockFromIRQ() {
    PortLock();
    dbg.CheckLockFromIRQ();
}

void UnlockFromIRQ() {
    dbg.CheckUnlockFromIRQ();
    PortUnlock();
}
#endif

void IrqPrologue() { dbg.CheckEnterIRQ(); }
void IrqEpilogue() {
    dbg.CheckLeaveIRQ();
    _port_irq_epilogue();
}

void DbgAssert(bool IsOk, const char *MsgIfNotOk) {
    dbg.Check(IsOk, MsgIfNotOk);
}
void DbgCheckClassI() { dbg.CheckClassI(); }

}  // namespace Sys

#if 1  // =========================== Hardware related
       // ==========================
void InitSysHw() {
    // ==== Init priority grouping ====
    NVIC_SetPriorityGrouping(7 - CORTEX_PRIORITY_BITS);
    // DWT cycle counter enable, note, the M7 requires DWT unlocking
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if CORTEX_MODEL == 7
    DWT->LAR = 0xC5ACCE55U;
#endif
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // Initialization of the system vectors used by the port
#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
    NVIC_SetPriority(SVCall_IRQn, CORTEX_PRIORITY_SVCALL);
#endif
    NVIC_SetPriority(PendSV_IRQn, CORTEX_PRIORITY_PENDSV);
    SysTimer::Init();
}

void EnableRtos() {
#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
    __set_BASEPRI(CORTEX_BASEPRI_DISABLED);
#endif
    __enable_irq();
}

void PortLock() {
#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
    __set_BASEPRI(CORTEX_BASEPRI_KERNEL);
#else  /* CORTEX_SIMPLIFIED_PRIORITY */
    __disable_irq();
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
}

void PortUnlock() {
#if CORTEX_SIMPLIFIED_PRIORITY == FALSE
    __set_BASEPRI(CORTEX_BASEPRI_DISABLED);
#else  /* CORTEX_SIMPLIFIED_PRIORITY */
    __enable_irq();
#endif /* CORTEX_SIMPLIFIED_PRIORITY */
}

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects directly the context
 *          switch performance so optimize here as much as you can.
 *
 * @param[in] newtp       the thread to be switched in
 * @param[in] oldtp       the thread to be switched out
 */
inline void SwitchContext(Thread *newtp, Thread *oldtp) {
    McuIntCtx *r13 = (McuIntCtx *)__get_PSP();
    if ((stkalign_t *)(r13 - 1) < oldtp->wabase) SysHalt("stack overflow");
    _port_switch(newtp, oldtp);
}

// Exception exit redirection to _port_switch_from_isr()
extern "C" void _port_irq_epilogue() {
    PortLock();
    if (SCB->ICSR & SCB_ICSR_RETTOBASE_Msk) {
        McuExtCtx_t *ctxp;
#if CORTEX_USE_FPU == \
    TRUE  // Enforcing a lazy FPU state save by accessing the FPCSR register
        (void)__get_FPSCR();
#endif
        ctxp = (McuExtCtx_t *)__get_PSP();
        // Adding an artificial exception return context, there is no need to
        // populate it fully
        ctxp--;

        // Set up a fake XPSR register value
        ctxp->xpsr = (regarm_t)0x01000000;
#if CORTEX_USE_FPU == TRUE
        ctxp->fpscr = (regarm_t)FPU->FPDSCR;
#endif
        // Write back the modified PSP value
        __set_PSP((uint32_t)ctxp);

        // The exit sequence is different depending on if a preemption is
        // required or not
        if (SchIsPreemptionRequired()) {  // Preemption is required => we need
                                          // to enforce a context switch
            ctxp->pc = (regarm_t)_port_switch_from_isr;
        } else {  // Preemption not required, we just need to exit the exception
                  // atomically
            ctxp->pc = (regarm_t)_port_exit_from_isr;
        }
        /* Note, returning without unlocking is intentional, this is done in
         order to keep the rest of the context switch atomic.*/
        return;
    }
    PortUnlock();
}

#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE)
// SVC IRQ handler. Used for exception mode re-entering after a context switch.
extern "C" void SVC_Handler() {
    McuExtCtx_t *ctxp;
#if CORTEX_USE_FPU
    // Enforce unstacking of the FP part of the context
    FPU->FPCCR &= ~FPU_FPCCR_LSPACT_Msk;
#endif
    // The McuExtCtx_t structure is pointed by the PSP register
    ctxp = (McuExtCtx_t *)__get_PSP();
    // Discard the current exception context and position the stack to point to
    // the real one
    ctxp++;
    // Restore real position of the original stack frame
    __set_PSP((uint32_t)ctxp);
    // Restore the normal interrupts status
    PortUnlock();
}
#endif /* CORTEX_SIMPLIFIED_PRIORITY == FALSE */

#if (CORTEX_SIMPLIFIED_PRIORITY == TRUE)
// PendSV vector used for exception mode re-entering after a context switch in
// compact kernel mode.
extern "C" void PendSV_Handler() {
    McuExtCtx_t *ctxp;
#if CORTEX_USE_FPU
    // Enforce unstacking of the FP part of the context
    FPU->FPCCR &= ~FPU_FPCCR_LSPACT_Msk;
#endif
    // The McuExtCtx_t structure is pointed by the PSP register
    ctxp = (McuExtCtx_t *)__get_PSP();
    // Discard the current exception context and position the stack to point to
    // the real one
    ctxp++;
    // Restore real position of the original stack frame
    __set_PSP((uint32_t)ctxp);
    // Restore the normal interrupts status
    PortUnlock();
}
#endif /* CORTEX_SIMPLIFIED_PRIORITY == TRUE */

#endif

#if 1  // ========================== Sys Timer
       // ==================================
#include "Serial.h"
namespace SysTimer {

inline void Init() {
    RCU->EnTimer(SYS_TIM);
    DBGMCU->StopTmrInDbg(
        SYS_TIM);  // En the stop mode during debug for this timer
    // Init the counter in free running mode
    uint32_t SysTimClk = Clk::GetTimInputFreq(SYS_TIM);
    SYS_TIM->PSC = (SysTimClk / SYS_TIM_FREQUENCY) - 1;
#if SYS_TIM_RESOLUTION == 16
    SYS_TIM->CAR = 0xFFFFUL;
#else  // 32 bit
    SYS_TIM->CAR = 0xFFFFFFFFUL;
#endif

    SYS_TIM->CHCTL0 = 0;
    SYS_TIM->CH0CV = 0;
    SYS_TIM->DMAINTEN = TIM_DMAINTEN_UPIE;
    SYS_TIM->CTL1 = 0;
    SYS_TIM->SWEVG = TIM_SWEVG_UPG; // Generate update event
    SYS_TIM->CTL0 = TIM_CTL0_CEN;
    // Enable IRQ
    Nvic::EnableVector(SYS_TIM_IRQn, SYS_TIM_IRQ_PRIORITY);
}

inline void StartAlarm(systime_t time) {
    SYS_TIM->CH0CV = (uint32_t)time;
    SYS_TIM->INTF = 0;
    SYS_TIM->DMAINTEN = TIM_DMAINTEN_CH0IE;
}

// inline systime_t GetCounter()        { return (systime_t)SYS_TIM->CNT; }
inline void StopAlarm() { SYS_TIM->DMAINTEN = 0; }
inline void SetAlarm(systime_t time) { SYS_TIM->CH0CV = (uint32_t)time; }
// inline systime_t GetAlarm()          { return (systime_t)SYS_TIM->CH0CV; }
// inline bool IsAlarmActive()          { return (SYS_TIM->DMAINTEN &
// TIM_DMAINTEN_CH0IE); }

}  // namespace SysTimer

static bool isFirstIRQ = 1;

// This interrupt is used for system tick
extern "C" void SYS_TIM_IRQ_HANDLER() {
    Sys::IrqPrologue();
    if (SYS_TIM->INTF) {
        uint32_t systim_flags = SYS_TIM->INTF;
        SYS_TIM->INTF = 0UL;
        if (systim_flags & TIM_INTF_CH0IF) {
            Sys::LockFromIRQ();
            VtDoTickI();
            Sys::UnlockFromIRQ();
        }

        if (systim_flags & TIM_INTF_UPIF) {
            // При инициализации таймера происходит вызов первого прерывания
            if (!isFirstIRQ) {
                Sys::Time_ += SYS_TIM->GetTopValue();
            }
            isFirstIRQ = 0;
        }
    }
    if (TIM0->INTF) {
        TIM0->INTF = 0;  // Сброс всех флагов
    }
    // if(SYS_TIM->INTF & TIM_INTF_CH0IF) {
    //     SYS_TIM->INTF = 0UL;
    //     Sys::LockFromIRQ();
    //     VtDoTickI();
    //     Sys::UnlockFromIRQ();
    // }
    Sys::IrqEpilogue();
}
#endif

extern "C" void _unhandled_exception() {
    PrintfI("%S\r", __FUNCTION__);
    while (true);
}
