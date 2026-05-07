/*
 * sysinit.S
 *
 *  Created on: 2023-07-11
 *      Author: laurelindo
 */

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif


#define SCB_VTOR                            0xE000ED08
#define SCB_CPACR                           0xE000ED88
#define SCB_FPCCR                           0xE000EF34
#define SCB_FPDSCR                          0xE000EF3C

#define FPCCR_ASPEN                         (1 << 31)
#define FPCCR_LSPEN                         (1 << 30)

#define CONTROL_MODE_PRIVILEGED             0
#define CONTROL_MODE_UNPRIVILEGED           1
#define CONTROL_USE_MSP                     0
#define CONTROL_USE_PSP                     2
#define CONTROL_FPCA                        4

#define CRT0_FPCCR_INIT                     (FPCCR_ASPEN | FPCCR_LSPEN)
#define CRT0_CPACR_INIT                     0x00F00000
#define CRT0_CONTROL_INIT                   (CONTROL_USE_PSP | CONTROL_MODE_PRIVILEGED)

#define CRT0_STACKS_FILL_PATTERN            0x55555555

	.syntax unified
	.cpu    cortex-m4
    .fpu    fpv4-sp-d16
	.thumb
	.text

    .align  2
    .thumb_func
	.global _start

_start:
	cpsid   i  /* Interrupts are globally masked initially.*/
	/* ==== Stack pointers init ==== */
    /* MSP stack pointers initialization.*/
    ldr     r0, =__main_stack_end__
    msr     MSP, r0

    /* PSP stack pointers initialization.*/
    ldr     r0, =__process_stack_end__
    msr     PSP, r0

	/* ==== VTOR init ==== */
    ldr     r0, =_vectors
    movw    r1, #SCB_VTOR & 0xFFFF
    movt    r1, #SCB_VTOR >> 16
    str     r0, [r1]

#if 1 /* ========= FPU init ========== */
    /* FPU FPCCR initialization.*/
    movw    r0, #CRT0_FPCCR_INIT & 0xFFFF
    movt    r0, #CRT0_FPCCR_INIT >> 16
    movw    r1, #SCB_FPCCR & 0xFFFF
    movt    r1, #SCB_FPCCR >> 16
    str     r0, [r1]
    dsb
    isb

    /* CPACR init */
    movw    r0, #CRT0_CPACR_INIT & 0xFFFF
    movt    r0, #CRT0_CPACR_INIT >> 16
    movw    r1, #SCB_CPACR & 0xFFFF
    movt    r1, #SCB_CPACR >> 16
    str     r0, [r1]
    dsb
    isb

    /* FPU FPSCR initially cleared.*/
    mov     r0, #0
    vmsr    FPSCR, r0

    /* FPU FPDSCR initially cleared.*/
    movw    r1, #SCB_FPDSCR & 0xFFFF
    movt    r1, #SCB_FPDSCR >> 16
    str     r0, [r1]

    /* Enforcing FPCA bit in the CONTROL register.*/
    movs    r0, #CRT0_CONTROL_INIT | CONTROL_FPCA

#else
    movs    r0, #CRT0_CONTROL_INIT
#endif

	/* CONTROL register initialization as configured.*/
    msr     CONTROL, r0
    isb

	/* ==== Stacks init ==== */
    ldr     r0, =CRT0_STACKS_FILL_PATTERN
    /* Main Stack initialization. Note, it assumes that the
       stack size is a multiple of 4 so the linker file must
       ensure this.*/
    ldr     r1, =__main_stack_base__
    ldr     r2, =__main_stack_end__
msloop:
    cmp     r1, r2
    itt     lo
    strlo   r0, [r1], #4
    blo     msloop

    /* Process Stack initialization. Note, it assumes that the
       stack size is a multiple of 4 so the linker file must
       ensure this.*/
    ldr     r1, =__process_stack_base__
    ldr     r2, =__process_stack_end__
psloop:
    cmp     r1, r2
    itt     lo
    strlo   r0, [r1], #4
    blo     psloop

    /* ==== Data init ====
    Note, it assumes that the DATA size is a multiple of 4 so the linker file must ensure this.*/
    ldr     r1, =_textdata_start
    ldr     r2, =_data_start
    ldr     r3, =_data_end
dloop:
    cmp     r2, r3
    ittt    lo
    ldrlo   r0, [r1], #4
    strlo   r0, [r2], #4
    blo     dloop

    /* ==== BSS init ====
    Note, it assumes that the DATA size is a multiple of 4 so the linker file must ensure this.*/
    movs    r0, #0
    ldr     r1, =_bss_start
    ldr     r2, =_bss_end
bloop:
    cmp     r1, r2
    itt     lo
    strlo   r0, [r1], #4
    blo     bloop

	/* ==== Constructors invocation ==== */
    ldr     r4, =__init_array_start
    ldr     r5, =__init_array_end
initloop:
    cmp     r4, r5
    bge     endinitloop
    ldr     r1, [r4], #4
    blx     r1
    b       initloop
endinitloop:

	/* ==== Jump to main ==== */
    b		main
