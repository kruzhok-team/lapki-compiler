#include "yartos_cfg.h"
#include "board.h"

#define CONTEXT_OFFSET  12 /* size of Thread Context */

.set    SCB_ICSR, 0xE000ED04
.set    ICSR_PENDSVSET, 0x10000000
.syntax unified
.cpu    cortex-m4
#if CORTEX_USE_FPU
	.fpu    fpv4-sp-d16
#else
    .fpu    softvfp
#endif
.thumb
.text

/*--------------------------------------------------------------------------*
 * Performs a context switch between two threads.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_switch
_port_switch:
                push    {r4, r5, r6, r7, r8, r9, r10, r11, lr}
#if CORTEX_USE_FPU
                vpush   {s16-s31}
#endif

                str     sp, [r1, #CONTEXT_OFFSET]
#if (CORTEX_SIMPLIFIED_PRIORITY == FALSE) && ((CORTEX_MODEL == 3) || (CORTEX_MODEL == 4))
                /* Workaround for ARM errata 752419, only applied if condition exists for it to be triggered.*/
                ldr     r3, [r0, #CONTEXT_OFFSET]
                mov     sp, r3
#else
                ldr     sp, [r0, #CONTEXT_OFFSET]
#endif

#if CORTEX_USE_FPU
                vpop    {s16-s31}
#endif
                pop     {r4, r5, r6, r7, r8, r9, r10, r11, pc}

/*--------------------------------------------------------------------------*
 * Start a thread by invoking its work function.
 *
 * Threads execution starts here, the code leaves the system critical zone
 * and then jumps into the thread function passed in register R4. The
 * register R5 contains the thread parameter. The function chThdExit() is
 * called on thread function return.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_thread_start
_port_thread_start:
                bl      _dbg_check_unlock
#if CORTEX_SIMPLIFIED_PRIORITY
                cpsie   i
#else
                movs    r3, #0              /* CORTEX_BASEPRI_DISABLED */
                msr     BASEPRI, r3
#endif
                mov     r0, r5
                blx     r4
                mov     r3, #0
                bl      SysHalt

/*--------------------------------------------------------------------------*
 * Post-IRQ switch code.
 *
 * Exception handlers return here for context switching.
 *--------------------------------------------------------------------------*/
                .thumb_func
                .globl  _port_switch_from_isr
_port_switch_from_isr:
                bl      _dbg_check_lock
                bl      SchDoRescheduleI
                bl      _dbg_check_unlock
                .globl  _port_exit_from_isr
_port_exit_from_isr:
#if CORTEX_SIMPLIFIED_PRIORITY
                movw    r3, #:lower16:SCB_ICSR
                movt    r3, #:upper16:SCB_ICSR
                mov     r2, ICSR_PENDSVSET
                str     r2, [r3, #0]
                cpsie   i
#else /* !CORTEX_SIMPLIFIED_PRIORITY */
                svc     #0
#endif /* !CORTEX_SIMPLIFIED_PRIORITY */
.L1:            b       .L1
