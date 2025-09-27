/*
* IRQs file for GD32E11x MCU
* Author: Kreyl
* Date: 2023-07-10
*/

        .syntax unified
        .cpu    cortex-m0
        .thumb

/* ================================== IRQs ================================== */
        .section    .vectors, "ax"
        .align      4
        .globl      _vectors
_vectors:
        .long       __main_stack_end__
        .long       Reset_Handler		/* 0x04 */
        .long       NMI_Handler			/* 0x08 */
        .long       HardFault_Handler	/* 0x0C */
        .long       MemManage_Handler	/* 0x10 */
        .long       BusFault_Handler	/* 0x14 */
        .long       UsageFault_Handler	/* 0x18 */
        .long       IrqRsrvd1C
        .long       IrqRsrvd20
        .long       IrqRsrvd24
        .long       IrqRsrvd28
        .long       SVC_Handler
        .long       DebugMon_Handler
        .long       IrqRsrvd34
        .long       PendSV_Handler
        .long       SysTick_Handler
        /* ==== External Interrupts ==== */
        .long       WWDGT_IRQHandler          			/* 16:Window Watchdog Timer */
        .long       LVD_IRQHandler                      /* 17:LVD through EXTI Line detect */
        .long       TAMPER_IRQHandler                   /* 18:Tamper through EXTI Line detect */
        .long       RTC_IRQHandler                      /* 19:RTC through EXTI Line */
        .long       FMC_IRQHandler                      /* 20:FMC */
        .long       RCU_CTC_IRQHandler                  /* 21:RCU and CTC */
        .long       EXTI0_IRQHandler                    /* 22:EXTI Line 0 */
        .long       EXTI1_IRQHandler                    /* 23:EXTI Line 1 */
        .long       EXTI2_IRQHandler                    /* 24:EXTI Line 2 */
        .long       EXTI3_IRQHandler                    /* 25:EXTI Line 3 */
        .long       EXTI4_IRQHandler                    /* 26:EXTI Line 4 */
        .long       DMA0_Channel0_IRQHandler            /* 27:DMA0 Channel0 */
        .long       DMA0_Channel1_IRQHandler            /* 28:DMA0 Channel1 */
        .long       DMA0_Channel2_IRQHandler            /* 29:DMA0 Channel2 */
        .long       DMA0_Channel3_IRQHandler            /* 30:DMA0 Channel3 */
        .long       DMA0_Channel4_IRQHandler            /* 31:DMA0 Channel4 */
        .long       DMA0_Channel5_IRQHandler            /* 32:DMA0 Channel5 */
        .long       DMA0_Channel6_IRQHandler            /* 33:DMA0 Channel6 */
        .long       ADC0_1_IRQHandler                   /* 34:ADC0 and ADC1 */
        .long       IrqRsrvd8C							/* 35 Reserved */
        .long       IrqRsrvd90							/* 36 Reserved */
        .long       IrqRsrvd94							/* 37 Reserved */
        .long       IrqRsrvd98							/* 38 Reserved */
        .long       EXTI5_9_IRQHandler                  /* 39:EXTI5 to EXTI9 */
        .long       TIMER0_BRK_TIMER8_IRQHandler        /* 40:TIMER0 Break and TIMER8 */
        .long       TIMER0_UP_TIMER9_IRQHandler         /* 41:TIMER0 Update and TIMER9 */
        .long       TIMER0_TRG_CMT_TIMER10_IRQHandler   /* 42:TIMER0 Trigger and Commutation and TIMER10 */
        .long       TIMER0_Channel_IRQHandler           /* 43:TIMER0 Channel Capture Compare */
        .long       TIMER1_IRQHandler                   /* 44:TIMER1 */
        .long       TIMER2_IRQHandler                   /* 45:TIMER2 */
        .long       TIMER3_IRQHandler                   /* 46:TIMER3 */
        .long       I2C0_EV_IRQHandler                  /* 47:I2C0 Event */
        .long       I2C0_ER_IRQHandler                  /* 48:I2C0 Error */
        .long       I2C1_EV_IRQHandler                  /* 49:I2C1 Event */
        .long       I2C1_ER_IRQHandler                  /* 50:I2C1 Error */
        .long       SPI0_IRQHandler                     /* 51:SPI0 */
        .long       SPI1_IRQHandler                     /* 52:SPI1 */
        .long       USART0_IRQHandler                   /* 53:USART0 */
        .long       USART1_IRQHandler                   /* 54:USART1 */
        .long       USART2_IRQHandler                   /* 55:USART2 */
        .long       EXTI10_15_IRQHandler                /* 56:EXTI10 to EXTI15 */
        .long       RTC_Alarm_IRQHandler                /* 57:RTC Alarm */
        .long       USBFS_WKUP_IRQHandler               /* 58:USBFS Wakeup */
        .long       TIMER7_BRK_TIMER11_IRQHandler       /* 59:TIMER7 Break and TIMER11 */
        .long       TIMER7_UP_TIMER12_IRQHandler        /* 60:TIMER7 Update and TIMER12 */
        .long       TIMER7_TRG_CMT_TIMER13_IRQHandler   /* 61:TIMER7 Trigger and Commutation and TIMER13 */
        .long       TIMER7_Channel_IRQHandler           /* 62:TIMER7 Channel Capture Compare */
        .long       IrqRsrvdFC							/* 63 Reserved */
        .long       EXMC_IRQHandler                     /* 64:EXMC */
        .long       IrqRsrvd104   						/* 65 Reserved */
        .long       TIMER4_IRQHandler                   /* 66:TIMER4 */
        .long       SPI2_IRQHandler                     /* 67:SPI2 */
        .long       UART3_IRQHandler                    /* 68:UART3 */
        .long       UART4_IRQHandler                    /* 69:UART4 */
        .long       TIMER5_IRQHandler                   /* 70:TIMER5 */
        .long       TIMER6_IRQHandler                   /* 71:TIMER6 */
        .long       DMA1_Channel0_IRQHandler            /* 72:DMA1 Channel0 */
        .long       DMA1_Channel1_IRQHandler            /* 73:DMA1 Channel1 */
        .long       DMA1_Channel2_IRQHandler            /* 74:DMA1 Channel2 */
        .long       DMA1_Channel3_IRQHandler            /* 75:DMA1 Channel3 */
        .long       DMA1_Channel4_IRQHandler            /* 76:DMA1 Channel4 */
        .long       IrqRsrvd134							/* 77 Reserved */
        .long       IrqRsrvd138							/* 78 Reserved */
        .long       IrqRsrvd13C							/* 79 Reserved */
        .long       IrqRsrvd140							/* 80 Reserved */
        .long       IrqRsrvd144							/* 81 Reserved */
		.long       IrqRsrvd148							/* 82 Reserved */
        .long       USBFS_IRQHandler                    /* 83 USBFS */

/* ============================= Irq Handlers =============================== */
        .text

        .align      2
        .thumb_func
        .weak       Reset_Handler
Reset_Handler:
		b 			_start

        .thumb_func
        .weak       NMI_Handler
        .weak       HardFault_Handler
        .weak       MemManage_Handler
        .weak       BusFault_Handler
        .weak       UsageFault_Handler
        .weak       IrqRsrvd1C
        .weak       IrqRsrvd20
        .weak       IrqRsrvd24
        .weak       IrqRsrvd28
        .weak       SVC_Handler
        .weak       DebugMon_Handler
        .weak       IrqRsrvd34
        .weak       PendSV_Handler
        .weak       SysTick_Handler
        /* External Interrupts */
		.weak 		WWDGT_IRQHandler
		.weak 		LVD_IRQHandler
		.weak 		TAMPER_IRQHandler
		.weak 		RTC_IRQHandler
		.weak 		FMC_IRQHandler
		.weak 		RCU_CTC_IRQHandler
		.weak 		EXTI0_IRQHandler
		.weak 		EXTI1_IRQHandler
		.weak 		EXTI2_IRQHandler
		.weak 		EXTI3_IRQHandler
		.weak 		EXTI4_IRQHandler
		.weak 		DMA0_Channel0_IRQHandler
		.weak 		DMA0_Channel1_IRQHandler
		.weak 		DMA0_Channel2_IRQHandler
		.weak 		DMA0_Channel3_IRQHandler
		.weak 		DMA0_Channel4_IRQHandler
		.weak 		DMA0_Channel5_IRQHandler
		.weak 		DMA0_Channel6_IRQHandler
		.weak 		ADC0_1_IRQHandler
		.weak 		IrqRsrvd8C
		.weak 		IrqRsrvd90
		.weak 		IrqRsrvd94
		.weak 		IrqRsrvd98
		.weak 		EXTI5_9_IRQHandler
		.weak 		TIMER0_BRK_TIMER8_IRQHandler
		.weak 		TIMER0_UP_TIMER9_IRQHandler
		.weak 		TIMER0_TRG_CMT_TIMER10_IRQHandler
		.weak 		TIMER0_Channel_IRQHandler
		.weak 		TIMER1_IRQHandler
		.weak 		TIMER2_IRQHandler
		.weak 		TIMER3_IRQHandler
		.weak 		I2C0_EV_IRQHandler
		.weak 		I2C0_ER_IRQHandler
		.weak 		I2C1_EV_IRQHandler
		.weak 		I2C1_ER_IRQHandler
		.weak 		SPI0_IRQHandler
		.weak 		SPI1_IRQHandler
		.weak 		USART0_IRQHandler
		.weak 		USART1_IRQHandler
		.weak 		USART2_IRQHandler
		.weak 		EXTI10_15_IRQHandler
		.weak 		RTC_Alarm_IRQHandler
		.weak 		USBFS_WKUP_IRQHandler
		.weak 		TIMER7_BRK_TIMER11_IRQHandler
		.weak 		TIMER7_UP_TIMER12_IRQHandler
		.weak 		TIMER7_TRG_CMT_TIMER13_IRQHandler
		.weak 		TIMER7_Channel_IRQHandler
		.weak 		IrqRsrvdFC
		.weak 		EXMC_IRQHandler
		.weak 		IrqRsrvd104
		.weak 		TIMER4_IRQHandler
		.weak 		SPI2_IRQHandler
		.weak 		UART3_IRQHandler
		.weak 		UART4_IRQHandler
		.weak 		TIMER5_IRQHandler
		.weak 		TIMER6_IRQHandler
		.weak 		DMA1_Channel0_IRQHandler
		.weak 		DMA1_Channel1_IRQHandler
		.weak 		DMA1_Channel2_IRQHandler
		.weak 		DMA1_Channel3_IRQHandler
		.weak 		DMA1_Channel4_IRQHandler
		.weak 		IrqRsrvd134
		.weak 		IrqRsrvd138
		.weak 		IrqRsrvd13C
		.weak 		IrqRsrvd140
		.weak 		IrqRsrvd144
		.weak 		IrqRsrvd148
		.weak 		USBFS_IRQHandler

        .thumb_func
NMI_Handler:
        .thumb_func
HardFault_Handler:
        .thumb_func
MemManage_Handler:
        .thumb_func
BusFault_Handler:
        .thumb_func
UsageFault_Handler:
        .thumb_func
IrqRsrvd1C:
        .thumb_func
IrqRsrvd20:
        .thumb_func
IrqRsrvd24:
        .thumb_func
IrqRsrvd28:
        .thumb_func
SVC_Handler:
        .thumb_func
DebugMon_Handler:
        .thumb_func
IrqRsrvd34:
        .thumb_func
PendSV_Handler:
        .thumb_func
SysTick_Handler:

/* External Interrupts */
        .thumb_func
WWDGT_IRQHandler:
        .thumb_func
LVD_IRQHandler:
        .thumb_func
TAMPER_IRQHandler:
        .thumb_func
RTC_IRQHandler:
        .thumb_func
FMC_IRQHandler:
        .thumb_func
RCU_CTC_IRQHandler:
        .thumb_func
EXTI0_IRQHandler:
        .thumb_func
EXTI1_IRQHandler:
        .thumb_func
EXTI2_IRQHandler:
        .thumb_func
EXTI3_IRQHandler:
        .thumb_func
EXTI4_IRQHandler:
        .thumb_func
DMA0_Channel0_IRQHandler:
        .thumb_func
DMA0_Channel1_IRQHandler:
        .thumb_func
DMA0_Channel2_IRQHandler:
        .thumb_func
DMA0_Channel3_IRQHandler:
        .thumb_func
DMA0_Channel4_IRQHandler:
        .thumb_func
DMA0_Channel5_IRQHandler:
        .thumb_func
DMA0_Channel6_IRQHandler:
        .thumb_func
ADC0_1_IRQHandler:
        .thumb_func
IrqRsrvd8C:
        .thumb_func
IrqRsrvd90:
        .thumb_func
IrqRsrvd94:
        .thumb_func
IrqRsrvd98:
        .thumb_func
EXTI5_9_IRQHandler:
        .thumb_func
TIMER0_BRK_TIMER8_IRQHandler:
        .thumb_func
TIMER0_UP_TIMER9_IRQHandler:
        .thumb_func
TIMER0_TRG_CMT_TIMER10_IRQHandler:
        .thumb_func
TIMER0_Channel_IRQHandler:
        .thumb_func
TIMER1_IRQHandler:
        .thumb_func
TIMER2_IRQHandler:
        .thumb_func
TIMER3_IRQHandler:
        .thumb_func
I2C0_EV_IRQHandler:
        .thumb_func
I2C0_ER_IRQHandler:
        .thumb_func
I2C1_EV_IRQHandler:
        .thumb_func
I2C1_ER_IRQHandler:
        .thumb_func
SPI0_IRQHandler:
        .thumb_func
SPI1_IRQHandler:
        .thumb_func
USART0_IRQHandler:
        .thumb_func
USART1_IRQHandler:
        .thumb_func
USART2_IRQHandler:
        .thumb_func
EXTI10_15_IRQHandler:
        .thumb_func
RTC_Alarm_IRQHandler:
        .thumb_func
USBFS_WKUP_IRQHandler:
        .thumb_func
TIMER7_BRK_TIMER11_IRQHandler:
        .thumb_func
TIMER7_UP_TIMER12_IRQHandler:
        .thumb_func
TIMER7_TRG_CMT_TIMER13_IRQHandler:
        .thumb_func
TIMER7_Channel_IRQHandler:
        .thumb_func
IrqRsrvdFC:
        .thumb_func
EXMC_IRQHandler:
        .thumb_func
IrqRsrvd104:
        .thumb_func
TIMER4_IRQHandler:
        .thumb_func
SPI2_IRQHandler:
        .thumb_func
UART3_IRQHandler:
        .thumb_func
UART4_IRQHandler:
        .thumb_func
TIMER5_IRQHandler:
        .thumb_func
TIMER6_IRQHandler:
        .thumb_func
DMA1_Channel0_IRQHandler:
        .thumb_func
DMA1_Channel1_IRQHandler:
        .thumb_func
DMA1_Channel2_IRQHandler:
        .thumb_func
DMA1_Channel3_IRQHandler:
        .thumb_func
DMA1_Channel4_IRQHandler:
        .thumb_func
IrqRsrvd134:
        .thumb_func
IrqRsrvd138:
        .thumb_func
IrqRsrvd13C:
        .thumb_func
IrqRsrvd140:
        .thumb_func
IrqRsrvd144:
        .thumb_func
IrqRsrvd148:
        .thumb_func
USBFS_IRQHandler:

        bl          _unhandled_exception

  #      .thumb_func
 #       .weak       _unhandled_exception
#_unhandled_exception:
#.stay:
#        b           .stay
