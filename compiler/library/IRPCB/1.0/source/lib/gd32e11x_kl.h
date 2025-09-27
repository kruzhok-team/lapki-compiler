/*
 * gd32e11x_kl.h
 *
 *  Created on: 12 июл. 2023 г.
 *      Author: laurelindo
 */

#ifndef LIB_GD32E11X_KL_H_
#define LIB_GD32E11X_KL_H_

#include "types.h"
#include "board.h"

#define GET_BITS(reg, mask, offset)         ((reg & (mask << offset)) >> offset)
#define SET_BITS(reg, mask, value, offset)  reg = ((reg & (~(mask << offset))) | (value << offset))
#define BITS_EQUAL(reg, mask, value, offset) (((reg >> offset) & mask) == value)

#define IRC48M_Hz       48000000UL
#define IRC8M_Hz        8000000UL
#define IRC40K_Hz       40000UL
#define LXTAL_Hz        32768UL

// define interrupt number
typedef enum IRQn {
    // Cortex-M4 processor exceptions numbers
    NonMaskableInt_IRQn          = -14,    /*!< 2 non maskable interrupt                                 */
    MemoryManagement_IRQn        = -12,    /*!< 4 Cortex-M4 memory management interrupt                  */
    BusFault_IRQn                = -11,    /*!< 5 Cortex-M4 bus fault interrupt                          */
    UsageFault_IRQn              = -10,    /*!< 6 Cortex-M4 usage fault interrupt                        */
    SVCall_IRQn                  = -5,     /*!< 11 Cortex-M4 SV call interrupt                           */
    DebugMonitor_IRQn            = -4,     /*!< 12 Cortex-M4 debug monitor interrupt                     */
    PendSV_IRQn                  = -2,     /*!< 14 Cortex-M4 pend SV interrupt                           */
    SysTick_IRQn                 = -1,     /*!< 15 Cortex-M4 system tick interrupt                       */
    // interrupt numbers
    WWDGT_IRQn                   = 0,      /*!< window watchDog timer interrupt                          */
    LVD_IRQn                     = 1,      /*!< LVD through EXTI line detect interrupt                   */
    TAMPER_IRQn                  = 2,      /*!< tamper through EXTI line detect                          */
    RTC_IRQn                     = 3,      /*!< RTC through EXTI line interrupt                          */
    FMC_IRQn                     = 4,      /*!< FMC interrupt                                            */
    RCU_CTC_IRQn                 = 5,      /*!< RCU and CTC interrupt                                    */
    EXTI0_IRQn                   = 6,      /*!< EXTI line 0 interrupts                                   */
    EXTI1_IRQn                   = 7,      /*!< EXTI line 1 interrupts                                   */
    EXTI2_IRQn                   = 8,      /*!< EXTI line 2 interrupts                                   */
    EXTI3_IRQn                   = 9,      /*!< EXTI line 3 interrupts                                   */
    EXTI4_IRQn                   = 10,     /*!< EXTI line 4 interrupts                                   */
    DMA0_Channel0_IRQn           = 11,     /*!< DMA0 channel0 interrupt                                  */
    DMA0_Channel1_IRQn           = 12,     /*!< DMA0 channel1 interrupt                                  */
    DMA0_Channel2_IRQn           = 13,     /*!< DMA0 channel2 interrupt                                  */
    DMA0_Channel3_IRQn           = 14,     /*!< DMA0 channel3 interrupt                                  */
    DMA0_Channel4_IRQn           = 15,     /*!< DMA0 channel4 interrupt                                  */
    DMA0_Channel5_IRQn           = 16,     /*!< DMA0 channel5 interrupt                                  */
    DMA0_Channel6_IRQn           = 17,     /*!< DMA0 channel6 interrupt                                  */
    ADC0_1_IRQn                  = 18,     /*!< ADC0 and ADC1 interrupt                                  */
    EXTI5_9_IRQn                 = 23,     /*!< EXTI[9:5] interrupts                                     */
    TIMER0_BRK_TIMER8_IRQn       = 24,     /*!< TIMER0 break and TIMER8 interrupts                       */
    TIMER0_UP_TIMER9_IRQn        = 25,     /*!< TIMER0 update and TIMER9 interrupts                      */
    TIMER0_TRG_CMT_TIMER10_IRQn  = 26,     /*!< TIMER0 trigger and commutation  and TIMER10 interrupts   */
    TIMER0_Channel_IRQn          = 27,     /*!< TIMER0 channel capture compare interrupts                */
    TIMER1_IRQn                  = 28,     /*!< TIMER1 interrupt                                         */
    TIMER2_IRQn                  = 29,     /*!< TIMER2 interrupt                                         */
    TIMER3_IRQn                  = 30,     /*!< TIMER3 interrupts                                        */
    I2C0_EV_IRQn                 = 31,     /*!< I2C0 event interrupt                                     */
    I2C0_ER_IRQn                 = 32,     /*!< I2C0 error interrupt                                     */
    I2C1_EV_IRQn                 = 33,     /*!< I2C1 event interrupt                                     */
    I2C1_ER_IRQn                 = 34,     /*!< I2C1 error interrupt                                     */
    SPI0_IRQn                    = 35,     /*!< SPI0 interrupt                                           */
    SPI1_IRQn                    = 36,     /*!< SPI1 interrupt                                           */
    USART0_IRQn                  = 37,     /*!< USART0 interrupt                                         */
    USART1_IRQn                  = 38,     /*!< USART1 interrupt                                         */
    USART2_IRQn                  = 39,     /*!< USART2 interrupt                                         */
    EXTI10_15_IRQn               = 40,     /*!< EXTI[15:10] interrupts                                   */
    RTC_ALARM_IRQn               = 41,     /*!< RTC alarm interrupt                                      */
    USBFS_WKUP_IRQn              = 42,     /*!< USBFS wakeup interrupt                                   */
    TIMER7_BRK_TIMER11_IRQn      = 43,     /*!< TIMER7 break and TIMER11 interrupts                      */
    TIMER7_UP_TIMER12_IRQn       = 44,     /*!< TIMER7 update and TIMER12 interrupts                     */
    TIMER7_TRG_CMT_TIMER13_IRQn  = 45,     /*!< TIMER7 trigger and commutation and TIMER13 interrupts    */
    TIMER7_Channel_IRQn          = 46,     /*!< TIMER7 channel capture compare interrupts                */
    EXMC_IRQn                    = 48,     /*!< EXMC global interrupt                                    */
    TIMER4_IRQn                  = 50,     /*!< TIMER4 global interrupt                                  */
    SPI2_IRQn                    = 51,     /*!< SPI2 global interrupt                                    */
    UART3_IRQn                   = 52,     /*!< UART3 global interrupt                                   */
    UART4_IRQn                   = 53,     /*!< UART4 global interrupt                                   */
    TIMER5_IRQn                  = 54,     /*!< TIMER5 global interrupt                                  */
    TIMER6_IRQn                  = 55,     /*!< TIMER6 global interrupt                                  */
    DMA1_Channel0_IRQn           = 56,     /*!< DMA1 channel0 global interrupt                           */
    DMA1_Channel1_IRQn           = 57,     /*!< DMA1 channel1 global interrupt                           */
    DMA1_Channel2_IRQn           = 58,     /*!< DMA1 channel2 global interrupt                           */
    DMA1_Channel3_IRQn           = 59,     /*!< DMA1 channel3 global interrupt                           */
    DMA1_Channel4_IRQn           = 60,     /*!< DMA1 channel3 global interrupt                           */
    USBFS_IRQn                   = 67,     /*!< USBFS global interrupt                                   */
} IRQn_Type;

// Main flash and SRAM memory map
#define FLASH_BASE          0x08000000UL
#define SRAM_BASE           0x20000000UL
#define OB_BASE             0x1FFFF800UL
#define DBG_BASE            0xE0042000UL // DBG MCU
#define EXMC_BASE           0xA0000000UL

// AHB1 memory map
#define USBFS_BASE          0x50000000UL
#define CRC_BASE            0x40023000UL
#define FMC_BASE            0x40022000UL
#define RCU_BASE            0x40021000UL

#define DMA0_BASE           0x40020000UL
#define DMA0_Channel0_BASE  (DMA0_BASE + 0x00000008UL)
#define DMA0_Channel1_BASE  (DMA0_BASE + 0x0000001CUL)
#define DMA0_Channel2_BASE  (DMA0_BASE + 0x00000030UL)
#define DMA0_Channel3_BASE  (DMA0_BASE + 0x00000044UL)
#define DMA0_Channel4_BASE  (DMA0_BASE + 0x00000058UL)
#define DMA0_Channel5_BASE  (DMA0_BASE + 0x0000006CUL)
#define DMA0_Channel6_BASE  (DMA0_BASE + 0x00000080UL)
#define DMA1_BASE           0x40020400UL
#define DMA1_Channel0_BASE  (DMA1_BASE + 0x00000008UL)
#define DMA1_Channel1_BASE  (DMA1_BASE + 0x0000001CUL)
#define DMA1_Channel2_BASE  (DMA1_BASE + 0x00000030UL)
#define DMA1_Channel3_BASE  (DMA1_BASE + 0x00000044UL)
#define DMA1_Channel4_BASE  (DMA1_BASE + 0x00000058UL)

// APB1 memory map
#define CTC_BASE            0x4000C800UL
#define DAC_BASE            0x40007400UL
#define PMU_BASE            0x40007000UL
#define BKP_BASE            0x40006C00UL
#define I2C1_BASE           0x40005800UL
#define I2C0_BASE           0x40005400UL
#define UART4_BASE          0x40005000UL
#define UART3_BASE          0x40004C00UL
#define USART2_BASE         0x40004800UL
#define USART1_BASE         0x40004400UL
#define SPI2_BASE           0x40003C00UL
#define SPI1_BASE           0x40003800UL
#define FWDGT_BASE          0x40003000UL
#define WWDGT_BASE          0x40002C00UL
#define RTC_BASE            0x40002800UL
#define TIMER13_BASE        0x40002000UL
#define TIMER12_BASE        0x40001C00UL
#define TIMER11_BASE        0x40001800UL
#define TIMER6_BASE         0x40001400UL
#define TIMER5_BASE         0x40001000UL
#define TIMER4_BASE         0x40000C00UL
#define TIMER3_BASE         0x40000800UL
#define TIMER2_BASE         0x40000400UL
#define TIMER1_BASE         0x40000000UL

// APB2 memory map
#define TIMER10_BASE        0x40015400UL
#define TIMER9_BASE         0x40015000UL
#define TIMER8_BASE         0x40014C00UL
#define USART0_BASE         0x40013800UL
#define TIMER7_BASE         0x40013400UL
#define SPI0_BASE           0x40013000UL
#define TIMER0_BASE         0x40012C00UL
#define ADC1_BASE           0x40012800UL
#define ADC0_BASE           0x40012400UL
#define GPIOE_BASE          0x40011800UL
#define GPIOD_BASE          0x40011400UL
#define GPIOC_BASE          0x40011000UL
#define GPIOB_BASE          0x40010C00UL
#define GPIOA_BASE          0x40010800UL
#define EXTI_BASE           0x40010400UL
#define AFIO_BASE           0x40010000UL

#if 1 // =========================== GPIO ======================================
struct GPIO_TypeDef {
    volatile uint32_t CTL0;     /*!< 0x00 GPIO port control register 0 */
    volatile uint32_t CTL1;     /*!< 0x04 GPIO port control register 1 */
    volatile uint32_t ISTAT;    /*!< 0x08 GPIO port input status register */
    volatile uint32_t OCTL;     /*!< 0x0C GPIO port output control register */
    volatile uint32_t BOP;      /*!< 0x10 GPIO port bit operation register */
    volatile uint32_t BC;       /*!< 0x14 GPIO bit clear register */
    volatile uint32_t LOCK;     /*!< 0x18 GPIO port configuration lock register */
    volatile uint32_t _Reserved[8];
    volatile uint32_t SPD;      /*!< 0x3C GPIO port bit speed register */

    void SetCtlMode(uint32_t PinN, uint32_t CtlMode) {
        if(PinN < 8) {
            uint32_t Offset = PinN * 4;
            CTL0 = (CTL0 & ~(0b1111UL << Offset)) | (CtlMode << Offset);
        }
        else {
            uint32_t Offset = (PinN - 8) * 4;
            CTL1 = (CTL1 & ~(0b1111UL << Offset)) | (CtlMode << Offset);
        }
    }
};

/* GPIOx(x=A,B,C,D,E) definitions */
#define GPIOA   ((GPIO_TypeDef*)GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef*)GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef*)GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef*)GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef*)GPIOE_BASE)

#if 1 // ==== Pins ====
#define PA0    GPIOA, 0
#define PA1    GPIOA, 1
#define PA2    GPIOA, 2
#define PA3    GPIOA, 3
#define PA4    GPIOA, 4
#define PA5    GPIOA, 5
#define PA6    GPIOA, 6
#define PA7    GPIOA, 7
#define PA8    GPIOA, 8
#define PA9    GPIOA, 9
#define PA10    GPIOA, 10
#define PA11    GPIOA, 11
#define PA12    GPIOA, 12
#define PA13    GPIOA, 13
#define PA14    GPIOA, 14
#define PA15    GPIOA, 15

#define PB0    GPIOB, 0
#define PB1    GPIOB, 1
#define PB2    GPIOB, 2
#define PB3    GPIOB, 3
#define PB4    GPIOB, 4
#define PB5    GPIOB, 5
#define PB6    GPIOB, 6
#define PB7    GPIOB, 7
#define PB8    GPIOB, 8
#define PB9    GPIOB, 9
#define PB10    GPIOB, 10
#define PB11    GPIOB, 11
#define PB12    GPIOB, 12
#define PB13    GPIOB, 13
#define PB14    GPIOB, 14
#define PB15    GPIOB, 15

#define PC0    GPIOC, 0
#define PC1    GPIOC, 1
#define PC2    GPIOC, 2
#define PC3    GPIOC, 3
#define PC4    GPIOC, 4
#define PC5    GPIOC, 5
#define PC6    GPIOC, 6
#define PC7    GPIOC, 7
#define PC8    GPIOC, 8
#define PC9    GPIOC, 9
#define PC10    GPIOC, 10
#define PC11    GPIOC, 11
#define PC12    GPIOC, 12
#define PC13    GPIOC, 13
#define PC14    GPIOC, 14
#define PC15    GPIOC, 15

#define PD0    GPIOD, 0
#define PD1    GPIOD, 1
#define PD2    GPIOD, 2
#define PD3    GPIOD, 3
#define PD4    GPIOD, 4
#define PD5    GPIOD, 5
#define PD6    GPIOD, 6
#define PD7    GPIOD, 7
#define PD8    GPIOD, 8
#define PD9    GPIOD, 9
#define PD10    GPIOD, 10
#define PD11    GPIOD, 11
#define PD12    GPIOD, 12
#define PD13    GPIOD, 13
#define PD14    GPIOD, 14
#define PD15    GPIOD, 15

#define PE0    GPIOE, 0
#define PE1    GPIOE, 1
#define PE2    GPIOE, 2
#define PE3    GPIOE, 3
#define PE4    GPIOE, 4
#define PE5    GPIOE, 5
#define PE6    GPIOE, 6
#define PE7    GPIOE, 7
#define PE8    GPIOE, 8
#define PE9    GPIOE, 9
#define PE10    GPIOE, 10
#define PE11    GPIOE, 11
#define PE12    GPIOE, 12
#define PE13    GPIOE, 13
#define PE14    GPIOE, 14
#define PE15    GPIOE, 15
#endif

// AFIO registers definitions
struct AFIO_TypeDef {
    volatile uint32_t EC;      /*!< 0x00 AFIO event control register */
    volatile uint32_t PCF0;    /*!< 0x04 AFIO port configuration register 0 */
    volatile uint32_t EXTISS[4]; /*!< 0x08..0x14 AFIO port EXTI sources selection register 0..3 */
    volatile uint32_t _Reserved; // 0x18
    volatile uint32_t PCF1;    /*!< 0x1C AFIO port configuration register 1 */
    volatile uint32_t CPSCTL;  /*!< 0x20 AFIO IO compensation control register */

    void DisableJtagDP() { SET_BITS(PCF0, 0b111UL, 0b010UL, 24); }

    void RemapSPI0_PB345()  { PCF0 |= 1 << 0; } // Remap SPI0 from PA 5,6,7 to PB 3,4,5
    void RemapI2C0_PB89()   { PCF0 |= 1 << 1; } // Remap I2C0 from PB 6,7 to PB 8,9
    void RemapUSART0_PB67() { PCF0 |= 1 << 2; } // Remap USART0 from PA 9,10 to PB 6,7
    void RemapUSART1_PD56() { PCF0 |= 1 << 3; } // Remap USART1 from PA 2,3 to PD 5,6
    void RemapTim2_PB4501() { SET_BITS(PCF0, 0b11UL, 0b10UL, 10); } // PB 4,5,0,1
    void RemapTim2_PC6789() { PCF0 |= 0b11UL << 10; } // PC 6,7,8,9
};

#define AFIO   ((AFIO_TypeDef*)AFIO_BASE)
#endif // GPIO

#if 1 // =========================== EXTI ======================================
struct EXTI_TypeDef {
    volatile uint32_t INTEN; /*!< 0x00 Interrupt enable register */
    volatile uint32_t EVEN;  /*!< 0x04 Event enable register */
    volatile uint32_t RTEN;  /*!< 0x08 Rising edge trigger enable register */
    volatile uint32_t FTEN;  /*!< 0x0C Falling edge trigger enable register */
    volatile uint32_t SWIEV; /*!< 0x10 Software interrupt event register */
    volatile uint32_t PD;    /*!< 0x14 Pending register */
};

#define EXTI    ((EXTI_TypeDef*) EXTI_BASE)
#endif // EXTI

#if 1 // =========================== UART ======================================
/* USARTx_STAT0 */
#define USART_STAT0_PERR    (1UL << 0)
#define USART_STAT0_FERR    (1UL << 1)
#define USART_STAT0_NERR    (1UL << 2)
#define USART_STAT0_ORERR   (1UL << 3)
#define USART_STAT0_IDLEF   (1UL << 4)
#define USART_STAT0_RBNE    (1UL << 5)
#define USART_STAT0_TC      (1UL << 6)
#define USART_STAT0_TBE     (1UL << 7)
#define USART_STAT0_LBDF    (1UL << 8)
#define USART_STAT0_CTSF    (1UL << 9)

/* USARTx_CTL0 */
#define USART_CTL0_SBKCMD   (1UL << 0) // send break command
#define USART_CTL0_RWU      (1UL << 1) // receiver wakeup from mute mode
#define USART_CTL0_REN      (1UL << 2) // enable receiver
#define USART_CTL0_TEN      (1UL << 3) // enable transmitter
#define USART_CTL0_IDLEIE   (1UL << 4) // enable idle line detected interrupt
#define USART_CTL0_RBNEIE   (1UL << 5) // enable read data buffer not empty interrupt and overrun error interrupt
#define USART_CTL0_TCIE     (1UL << 6) // enable transmission complete interrupt
#define USART_CTL0_TBEIE    (1UL << 7) // enable transmitter buffer empty interrupt
#define USART_CTL0_PERRIE   (1UL << 8) // enable parity error interrupt
#define USART_CTL0_PM       (1UL << 9) // parity mode
#define USART_CTL0_PCEN     (1UL << 10) // enable parity check function
#define USART_CTL0_WM       (1UL << 11) // wakeup method in mute mode
#define USART_CTL0_WL       (1UL << 12) // word length
#define USART_CTL0_UEN      (1UL << 13) // enable USART

/* USARTx_CTL2 */
#define USART_CTL2_ERRIE    (1UL << 0) // enable error interrupt
#define USART_CTL2_IREN     (1UL << 1) // enable IrDA mode
#define USART_CTL2_IRLP     (1UL << 2) // IrDA low-power
#define USART_CTL2_HDEN     (1UL << 3) // enable half-duplex
#define USART_CTL2_NKEN     (1UL << 4) // mode NACK enable in smartcard
#define USART_CTL2_SCEN     (1UL << 5) // senable martcard mode
#define USART_CTL2_DENR     (1UL << 6) // enable DMA request for reception
#define USART_CTL2_DENT     (1UL << 7) // enable DMA request for transmission
#define USART_CTL2_RTSEN    (1UL << 8) // enable RTS
#define USART_CTL2_CTSEN    (1UL << 9) // enable CTS
#define USART_CTL2_CTSIE    (1UL << 10) // enable CTS interrupt

struct USART_TypeDef {
    volatile uint32_t STAT0; // 0
    volatile uint32_t DATA;  // 4
    volatile uint32_t BAUD;  // 8
    volatile uint32_t CTL0;  // C
    volatile uint32_t CTL1;  // 10
    volatile uint32_t CTL2;  // 14
    volatile uint32_t GP;    // 18
    volatile uint32_t _Reserved1[25];
    volatile uint32_t CTL3;  // 80
    volatile uint32_t RT;    // 84
    volatile uint32_t STAT1; // 88
    volatile uint32_t _Reserved2[13];
    volatile uint32_t CHC; // C0

    void EnableTx()  { CTL0 |=  USART_CTL0_TEN; }
    void DisableTx() { CTL0 &= ~USART_CTL0_TEN; }
    void EnableRx()  { CTL0 |=  USART_CTL0_REN; }
    void DisableRx() { CTL0 &= ~USART_CTL0_REN; }
};

#define USART0      ((USART_TypeDef*)USART0_BASE)
#define USART1      ((USART_TypeDef*)USART1_BASE)
#define USART2      ((USART_TypeDef*)USART2_BASE)
#define UART3       ((USART_TypeDef*)UART3_BASE)
#define UART4       ((USART_TypeDef*)UART4_BASE)

#endif // UART

#if 1 // ============================== DMA ====================================
struct DMABlock_t {
    volatile uint32_t INTF;
    volatile uint32_t INTC;
};

struct DMAChannel_t {
    volatile uint32_t CTL;
    volatile uint32_t CNT;
    volatile uint32_t CPADDR;
    volatile uint32_t CMADDR;
};

#define DMA0                ((DMABlock_t *)DMA0_BASE)
#define DMA1                ((DMABlock_t *)DMA1_BASE)

#define DMA0_Channel0       ((DMAChannel_t *)DMA0_Channel0_BASE)
#define DMA0_Channel1       ((DMAChannel_t *)DMA0_Channel1_BASE)
#define DMA0_Channel2       ((DMAChannel_t *)DMA0_Channel2_BASE)
#define DMA0_Channel3       ((DMAChannel_t *)DMA0_Channel3_BASE)
#define DMA0_Channel4       ((DMAChannel_t *)DMA0_Channel4_BASE)
#define DMA0_Channel5       ((DMAChannel_t *)DMA0_Channel5_BASE)
#define DMA0_Channel6       ((DMAChannel_t *)DMA0_Channel6_BASE)

#define DMA1_Channel0       ((DMAChannel_t *)DMA1_Channel0_BASE)
#define DMA1_Channel1       ((DMAChannel_t *)DMA1_Channel1_BASE)
#define DMA1_Channel2       ((DMAChannel_t *)DMA1_Channel2_BASE)
#define DMA1_Channel3       ((DMAChannel_t *)DMA1_Channel3_BASE)
#define DMA1_Channel4       ((DMAChannel_t *)DMA1_Channel4_BASE)

#define DMA_MEM2MEM_MODE        (1UL << 14)

#define DMA_PRIO_LOW            (0b00UL << 12)
#define DMA_PRIO_MEDIUM         (0b01UL << 12)
#define DMA_PRIO_HIGH           (0b10UL << 12)
#define DMA_PRIO_VERYHIGH       (0b11UL << 12)

#define DMA_MEMSZ_8_BIT         (0b00UL << 10)
#define DMA_MEMSZ_16_BIT        (0b01UL << 10)
#define DMA_MEMESZ_32_BIT       (0b10UL << 10)

#define DMA_PERSZ_8_BIT         (0b00UL << 8)
#define DMA_PERSZ_16_BIT        (0b01UL << 8)
#define DMA_PERSZ_32_BIT        (0b10UL << 8)

#define DMA_MEM_INC             (1UL << 7)
#define DMA_PER_INC             (1UL << 6)
#define DMA_CIRC                (1UL << 5)

#define DMA_DIR_MEM2PER         (1UL << 4)
#define DMA_DIR_PER2MEM         (0UL << 4)

#define DMA_ERRIE               (1UL << 3)
#define DMA_HTFIE               (1UL << 2)
#define DMA_TCIE                (1UL << 1)

#define DMA0_CHNL_CNT           (7UL)
#define DMA1_CHNL_CNT           (5UL)
#define DMA_CHNL_CNT            (DMA0_CHNL_CNT + DMA1_CHNL_CNT)
// Translate dma0 chnls[0;6] and dma1 chnls [0;4] to chnls[0; 12]
#define DMA_CHNL(dma, chnl)     ((dma) * 7 + (chnl))

#define DMA_CHNL_EN             (1UL << 0)
#endif // DMA

#if 1 // ============================= Timer ===================================
// Bits
#define TIM_CTL0_CEN        (1UL << 0)
#define TIM_CTL0_UPDIS      (1UL << 1)
#define TIM_CTL0_UPS        (1UL << 2)
#define TIM_CTL0_SPM        (1UL << 3)
#define TIM_CTL0_DIR        (1UL << 4)
#define TIM_CTL0_ARSE       (1UL << 7)
#define TIM_SMCFG_ETP       (1UL << 15)

#define TIM_DMAINTEN_UPIE   (1UL << 0)
#define TIM_DMAINTEN_CH0IE  (1UL << 1)
#define TIM_DMAINTEN_CH1IE  (1UL << 2)
#define TIM_DMAINTEN_CH2IE  (1UL << 3)
#define TIM_DMAINTEN_CH3IE  (1UL << 4)
#define TIM_DMAINTEN_TRGIE  (1UL << 6)
#define TIM_DMAINTEN_UPDEN  (1UL << 8)
#define TIM_DMAINTEN_CH0DEN (1UL << 9)
#define TIM_DMAINTEN_CH1DEN (1UL << 10)
#define TIM_DMAINTEN_CH2DEN (1UL << 11)
#define TIM_DMAINTEN_CH3DEN (1UL << 12)
#define TIM_DMAINTEN_DMAEN(chnl)    (1UL << (9UL + chnl))
#define TIM_DMAINTEN_TRGDEN (1UL << 14)

#define TIM_INTF_UPIF       (1UL << 0)
#define TIM_INTF_CH0IF      (1UL << 1)
#define TIM_INTF_CH1IF      (1UL << 2)
#define TIM_INTF_CH2IF      (1UL << 3)
#define TIM_INTF_CH3IF      (1UL << 4)
#define TIM_SWEVG_UPG       (1UL << 0)

struct TIM_TypeDef {
    volatile uint32_t CTL0;     /*!< 0x00 control register 1 */
    volatile uint32_t CTL1;     /*!< 0x04 control register 2 */
    volatile uint32_t SMCFG;    /*!< 0x08 slave mode control register */
    volatile uint32_t DMAINTEN; /*!< 0x0C DMA/interrupt enable register */
    volatile uint32_t INTF;     /*!< 0x10 status register */
    volatile uint32_t SWEVG;    /*!< 0x14 event generation register */
    volatile uint32_t CHCTL0;   /*!< 0x18 Channel control register 0 */
    volatile uint32_t CHCTL1;   /*!< 0x1C Channel control register 1 */
    volatile uint32_t CHCTL2;   /*!< 0x20 Channel control register 2 */
    volatile uint32_t CNT;      /*!< 0x24 counter register */
    volatile uint32_t PSC;      /*!< 0x28 prescaler */
    volatile uint32_t CAR;      /*!< 0x2C auto-reload register */
    volatile uint32_t CREP;     /*!< 0x30 repetition counter register */
    volatile uint32_t CH0CV;    /*!< 0x34 capture/compare value register 0 */
    volatile uint32_t CH1CV;    /*!< 0x38 capture/compare value register 1 */
    volatile uint32_t CH2CV;    /*!< 0x3C capture/compare value register 3 */
    volatile uint32_t CH3CV;    /*!< 0x40 capture/compare value register 4 */
    volatile uint32_t CCHP;     /*!< 0x44 Complementary channel protection register */
    volatile uint32_t DMACFG;   /*!< 0x48 DMA configuration register */
    volatile uint32_t DMATB;    /*!< 0x4C DMA transfer buffer register */
    volatile uint32_t _Reserved[43];
    volatile uint32_t CFG;      /*!< 0xFC Configuration register */

    void Enable()  { CTL0 |=  TIM_CTL0_CEN; }
    void Disable() { CTL0 &= ~TIM_CTL0_CEN; }
    void SetTopValue(uint32_t value) { CAR = value; }
    uint32_t GetTopValue() { return CAR; }
    void SetPrescaler(uint32_t psc) { PSC = psc; }
    uint32_t GetPrescaler() { return PSC; }
    void SetChnlValue(const uint32_t Chnl, uint32_t value) { *(uint32_t*)(&CH0CV + Chnl) = value; }
    void GenerateUpdateEvt() { SWEVG = (1UL << 0); }
};

// Timers
#define TIM0    ((TIM_TypeDef*)TIMER0_BASE)
#define TIM1    ((TIM_TypeDef*)TIMER1_BASE)
#define TIM2    ((TIM_TypeDef*)TIMER2_BASE)
#define TIM3    ((TIM_TypeDef*)TIMER3_BASE)
#define TIM4    ((TIM_TypeDef*)TIMER4_BASE)
#define TIM5    ((TIM_TypeDef*)TIMER5_BASE)
#define TIM6    ((TIM_TypeDef*)TIMER6_BASE)
#define TIM7    ((TIM_TypeDef*)TIMER7_BASE)
#define TIM8    ((TIM_TypeDef*)TIMER8_BASE)
#define TIM9    ((TIM_TypeDef*)TIMER9_BASE)
#define TIM10   ((TIM_TypeDef*)TIMER10_BASE)
#define TIM11   ((TIM_TypeDef*)TIMER11_BASE)
#define TIM12   ((TIM_TypeDef*)TIMER12_BASE)
#define TIM13   ((TIM_TypeDef*)TIMER13_BASE)

#define TIM_CNT    14 // [TIM0; TIM13]
#endif // timer

#if 1 // ======================= Free Watchdog =================================
struct FWDGT_TypeDef {
    volatile uint32_t CTL;  // 0x00 Control register
    volatile uint32_t PSC;  // 0x04 Prescaler register
    volatile uint32_t RLD;  // 0x08 Reload register
    volatile uint32_t STAT; // 0x0C Status register
};

#define FWDGT       ((FWDGT_TypeDef*)FWDGT_BASE)
#endif

#if 1 // ============================= SPI =====================================
#define SPI_CTL0_CKPH       (1UL << 0)
#define SPI_CTL0_CKPL       (1UL << 1)
#define SPI_CTL0_MSTMOD     (1UL << 2)
#define SPI_CTL0_SPIEN      (1UL << 6)
#define SPI_CTL0_LF         (1UL << 7)
#define SPI_CTL0_SWNSS      (1UL << 8)
#define SPI_CTL0_SWNSSEN    (1UL << 9)
#define SPI_CTL0_RO         (1UL << 10)
#define SPI_CTL0_FF16       (1UL << 11)

#define SPI_CTL1_DMAREN     (1UL << 0)
#define SPI_CTL1_DMATEN     (1UL << 1)
#define SPI_CTL1_NSSDRV     (1UL << 2)
#define SPI_CTL1_NSSP       (1UL << 3)
#define SPI_CTL1_ERRIE      (1UL << 5)
#define SPI_CTL1_RBNEIE     (1UL << 6)
#define SPI_CTL1_TBEIE      (1UL << 7)

#define SPI_STAT_RBNE       (1UL << 0)
#define SPI_STAT_TBE        (1UL << 1)
#define SPI_STAT_RXORERR    (1UL << 6)
#define SPI_STAT_TRANS      (1UL << 7) // Busy

#define I2S_CTL_CHLEN       (1UL << 0) // Sz of channel
#define I2S_CTL_CHLEN_16bit (0UL << 0)
#define I2S_CTL_CHLEN_32bit (1UL << 0)

#define I2S_CTL_DTLEN_msk   (0b11UL << 1) // Sz of payload data in channel
#define I2S_CTL_DTLEN_16bit (0b00UL << 1)
#define I2S_CTL_DTLEN_24bit (0b01UL << 1)
#define I2S_CTL_DTLEN_32bit (0b10UL << 1)

#define I2S_CTL_CKPL        (1UL << 3)
#define I2S_CTL_CKPL_IdleLo (0UL << 3)
#define I2S_CTL_CKPL_IdleHi (1UL << 3)

#define I2S_CTL_STD_msk     (0b11UL << 4)
#define I2S_CTL_STD_I2S     (0b00UL << 4)
#define I2S_CTL_STD_MSB     (0b01UL << 4)
#define I2S_CTL_STD_LSB     (0b10UL << 4)
#define I2S_CTL_STD_PCM     (0b11UL << 4)

#define I2S_CTL_SLAVE_TX    (0b00UL << 8)
#define I2S_CTL_SLAVE_RX    (0b01UL << 8)
#define I2S_CTL_MASTER_TX   (0b10UL << 8)
#define I2S_CTL_MASTER_RX   (0b11UL << 8)

#define I2S_CTL_I2SEN       (1UL << 10)
#define I2S_CTL_I2SSEL      (1UL << 11)

#define SPI_QCTL_IO23DR     (1UL << 2)
#define SPI_QCTL_QRD        (1UL << 1)
#define SPI_QCTL_QMOD       (1UL << 0)

struct SPI_TypeDef {
    volatile uint32_t CTL0;    /*!< Control register 0,      offset: 0x00 */
    volatile uint32_t CTL1;    /*!< Control register 1,      offset: 0x04 */
    volatile uint32_t STAT;    /*!< Status register,         offset: 0x08 */
    volatile uint32_t DATA;    /*!< Data register,           offset: 0x0C */
    volatile uint32_t CRCPOLY; /*!< CRC polynomial register, offset: 0x10 */
    volatile uint32_t RCRC;    /*!< RX CRC register,         offset: 0x14 */
    volatile uint32_t TCRC;    /*!< TX CRC register,         offset: 0x18 */
    volatile uint32_t I2SCTL;  /*!< I2S control register,    offset: 0x1C */
    volatile uint32_t I2SPSC;  /*!< I2S control register,    offset: 0x20 */
    volatile uint32_t resvd[23];
    volatile uint32_t QCTL;    /*!< Quad-SPI mode control register, Address offset: 0x80 */

    void Enable()  { CTL0 |=  SPI_CTL0_SPIEN; }
    void Disable() { CTL0 &= ~SPI_CTL0_SPIEN; }
    void EnableI2S()  { I2SCTL |=  I2S_CTL_I2SEN; }
    void DisableI2S() { I2SCTL &= ~I2S_CTL_I2SEN; }

    // IRQ
    void EnRxBufNotEmptyIrq()  { CTL1 |=  SPI_CTL1_RBNEIE; }
    void DisRxBufNotEmptyIrq() { CTL1 &= ~SPI_CTL1_RBNEIE; }

    // DMA
    void EnTxDma()    { CTL1 |=  SPI_CTL1_DMATEN; }
    void DisTxDma()   { CTL1 &= ~SPI_CTL1_DMATEN; }
    void EnRxDma()    { CTL1 |=  SPI_CTL1_DMAREN; }
    void DisRxDma()   { CTL1 &= ~SPI_CTL1_DMAREN; }
    void EnRxTxDma()  { CTL1 |=  (SPI_CTL1_DMATEN | SPI_CTL1_DMAREN); }
    void DisRxTxDma() { CTL1 &= ~(SPI_CTL1_DMATEN | SPI_CTL1_DMAREN); }

    // Flags and buf
    void WaitForTransLo() { while(STAT & SPI_STAT_TRANS); }
    void WaitForTBEHi()   { while(!(STAT & SPI_STAT_TBE)); }
    void WaitForRBNEHi()  { while(!(STAT & SPI_STAT_RBNE)); }
    void WaitForTBEHiAndTransLo() { while((STAT & (SPI_STAT_TBE | SPI_STAT_TRANS)) != SPI_STAT_TBE); }
    void ClearRxOvrErr()  { (void)DATA; (void)STAT; (void)DATA; }
};

#define SPI0    ((SPI_TypeDef*)SPI0_BASE)
#define SPI1    ((SPI_TypeDef*)SPI1_BASE)
#define SPI2    ((SPI_TypeDef*)SPI2_BASE)
#endif

#if 1 // =========================== Debug MCU =================================
struct DBGMCU_TypeDef {
    volatile uint32_t id;  /*!< 0x00 MCU device ID code */
    volatile uint32_t CTL; /*!< 0x04 Debug MCU configuration register */

    void StopTmrInDbg(const TIM_TypeDef *PTimer) {
        if     (PTimer == TIM0)  CTL |= 1UL << 10;
        else if(PTimer == TIM1)  CTL |= 1UL << 11;
        else if(PTimer == TIM2)  CTL |= 1UL << 12;
        else if(PTimer == TIM3)  CTL |= 1UL << 13;
        else if(PTimer == TIM4)  CTL |= 1UL << 17;
        else if(PTimer == TIM5)  CTL |= 1UL << 18;
        else if(PTimer == TIM6)  CTL |= 1UL << 19;
        else if(PTimer == TIM7)  CTL |= 1UL << 20;
        else if(PTimer == TIM8)  CTL |= 1UL << 28;
        else if(PTimer == TIM9)  CTL |= 1UL << 29;
        else if(PTimer == TIM10) CTL |= 1UL << 30;
        else if(PTimer == TIM11) CTL |= 1UL << 25;
        else if(PTimer == TIM12) CTL |= 1UL << 26;
        else if(PTimer == TIM13) CTL |= 1UL << 27;
    }
};

#define DBGMCU  ((DBGMCU_TypeDef*)DBG_BASE)
#endif

#if 1 // ===================== Reset and Clock Control =========================
// Clocks
#define RCU_IRC40K_FREQ_Hz      40000UL

// Bits
#define RCU_CTL_IRC8MEN         (1UL <<  0)
#define RCU_CTL_IRC8MSTB        (1UL <<  1)
#define RCU_CTL_HXTALEN         (1UL << 16)
#define RCU_CTL_HXTALSTB        (1UL << 17)
#define RCU_CTL_HXTALBPS        (1UL << 18)
#define RCU_CTL_CKMEN           (1UL << 19)
#define RCU_CTL_PLLEN           (1UL << 24)
#define RCU_CTL_PLLSTB          (1UL << 25)
#define RCU_CTL_PLL1EN          (1UL << 26)
#define RCU_CTL_PLL1STB         (1UL << 27)
#define RCU_CTL_PLL2EN          (1UL << 28)
#define RCU_CTL_PLL2STB         (1UL << 29)

#define RCU_CFG0_PLLSEL         (1UL << 16)
#define RCU_CFG1_PREDV0SEL      (1UL << 16)
#define RCU_CFG1_I2S1SEL        (1UL << 17)
#define RCU_CFG1_I2S2SEL        (1UL << 18)
#define RCU_CFG1_PLLPRESEL      (1UL << 30)

#define RCU_RSTSCK_IRC40KEN     (1UL << 0) // IRC40K enable
#define RCU_RSTSCK_IRC40KSTB    (1UL << 1) // IRC40K stabilization flag

#define RCU_ADDCTL_IRC48MEN     (1UL << 16)
#define RCU_ADDCTL_IRC48MSTB    (1UL << 17)
#define RCU_ADDCTL_CK48MSEL     (1UL <<  0)

#define CLK_STARTUP_TIMEOUT     99999UL  /*!< Simple loop delay count to allow clock to start */

enum class AhbPsc { div1=0, div2=0b1000, div4=0b1001 };
enum class PllMulti {
    mul02=0b00000, mul03=0b00001, mul04=0b00010, mul05=0b00011, mul06=0b00100, mul6d5=0b01101,
    mul07=0b00101, mul08=0b00110, mul09=0b00111, mul10=0b01000, mul11=0b01001,  mul12=0b01010,
    mul13=0b01011, mul14=0b01100, mul16=0b01110, mul17=0b10000, mul18=0b10001,  mul19=0b10010,
    mul20=0b10011, mul21=0b10100, mul22=0b10101, mul23=0b10110, mul24=0b10111,  mul25=0b11000,
    mul26=0b11001, mul27=0b11010, mul28=0b11011, mul29=0b11100, mul30=0b11101,  mul31=0b11110
};
enum class Pll2Multi {
    mul08=0b0110, mul09=0b0111, mul10=0b1000, mul11=0b1001,  mul12=0b1010,
    mul13=0b1011, mul14=0b1100, mul16=0b1110, mul20=0b1111
};
enum class UsbPsc : uint32_t {
    div1d5 = (0UL << 31) | (0b00UL << 22),
    div1   = (0UL << 31) | (0b01UL << 22),
    div2d5 = (0UL << 31) | (0b10UL << 22),
    div2   = (0UL << 31) | (0b11UL << 22),
    div3   = (1UL << 31) | (0b00UL << 22),
    div3d5 = (1UL << 31) | (0b01UL << 22),
    div4   = (1UL << 31) | (0b10UL << 22),
};

enum class AdcPsc : uint32_t {
    Apb2div2 = 0b0000, Apb2div4 = 0b0001, Apb2div6 = 0b0010, Apb2div8 = 0b0011,
    Apb2div12 = 0b0101, Apb2div16 = 0b0111,
    AHBdiv3 = 0b1000, AHBdiv5 = 0b1001, AHBdiv7 = 0b1010, AHBdiv9 = 0b1011
};

struct RCU_TypeDef {
    volatile uint32_t CTL;     // 0  Control register
    volatile uint32_t CFG0;    // 4  Clock configuration register 0
    volatile uint32_t INT;     // 8  Clock interrupt register
    volatile uint32_t APB2RST; // C
    volatile uint32_t APB1RST; // 10
    volatile uint32_t AHBEN;   // 14
    volatile uint32_t APB2EN;  // 18
    volatile uint32_t APB1EN;  // 1C
    volatile uint32_t BDCTL;   // 20
    volatile uint32_t RSTSCK;  // 24
    volatile uint32_t AHBRST;  // 28
    volatile uint32_t CFG1;    // 2C Clock configuration register 1
    volatile uint32_t _Reserved1;
    volatile uint32_t DSV;     // 34
    volatile uint32_t _Reserved2[34];
    volatile uint32_t ADDCTL;  // C0 Additional clock control register
    volatile uint32_t _Reserved3[2];
    volatile uint32_t ADDINT;  // CC
    volatile uint32_t _Reserved4[4];
    volatile uint32_t ADDAPB1RST; // E0
    volatile uint32_t ADDAPB1EN;  // E4

#if 1 // ==== XTAL, PLL etc. ====
    retv EnableIRC8M() {
        CTL |= RCU_CTL_IRC8MEN;
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(CTL & RCU_CTL_IRC8MSTB) return retv::Ok;
        return retv::Timeout;
    }

    retv EnableXTAL() {
        CTL |= RCU_CTL_HXTALEN; // Enable XTAL
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(CTL & RCU_CTL_HXTALSTB) return retv::Ok;
        return retv::Timeout;
    }

    retv EnablePll() {
        CTL |= RCU_CTL_PLLEN; // Enable PLL
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(CTL & RCU_CTL_PLLSTB) return retv::Ok;
        return retv::Timeout;
    }

    retv EnablePll2() {
        CTL |= RCU_CTL_PLL2EN; // Enable PLL
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(CTL & RCU_CTL_PLL2STB) return retv::Ok;
        return retv::Timeout;
    }

    retv EnableIRC48M() {
        ADDCTL |= RCU_ADDCTL_IRC48MEN;
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(ADDCTL & RCU_ADDCTL_IRC48MSTB) return retv::Ok;
        return retv::Timeout;
    }

    void DisableAllClksExIrc8M() {
        CTL &= 0x0000FFFFUL; // PLL2, PLL1, PLL, CKM, XTAL
        ADDCTL &= ~RCU_ADDCTL_IRC48MEN;
    }

    void DisablePll2() { CTL &= ~RCU_CTL_PLL2EN; }


    void SetPllPresel_XTAL()     { CFG1 &= ~(1UL << 30); }
    void SetPllPresel_CKIRC48M() { CFG1 |=  (1UL << 30); }

    void SetPrediv0Sel_XTALorIRC48M() { CFG1 &= ~(1UL << 16); }
    void SetPrediv0Sel_CKPLL1()       { CFG1 |=  (1UL << 16); }

    // Div = [1; 16]
    void SetPrediv0(uint32_t Div) { SET_BITS(CFG1, 0b1111UL, ((Div - 1UL) & 0b1111UL), 0); }
    void SetPrediv1(uint32_t Div) { SET_BITS(CFG1, 0b1111UL, ((Div - 1UL) & 0b1111UL), 4); }

    void SetPllSel_Irc8Mdiv2() { CFG0 &= ~(1UL << 16); }
    void SetPllSel_Prediv0()   { CFG0 |=  (1UL << 16); }

    void SetPllMulti(PllMulti Multi) {
        uint32_t tmp = CFG0 & ~((1UL << 29) | (0b1111UL << 18));
        uint32_t m = (uint32_t)Multi;
        tmp |= ((m & 0b01111UL) << 18) | ((m & 0b10000UL) << 25); // 25 to put 5-th bit to 29 position
        CFG0 = tmp;
    }
    void SetPll2Multi(Pll2Multi Multi) { SET_BITS(CFG1, 0b1111UL, (uint32_t)Multi, 12); }

    // CkSys Switch
    uint32_t GetCkSwitchState() { return (CFG0 >> 2) & 0b11UL; }
    retv Switch2IRC8M() {
        SET_BITS(CFG0, 0b11UL, 0b00UL, 0); // Select IRC8M
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(BITS_EQUAL(CFG0, 0b11UL, 0b00UL, 2)) return retv::Ok;
        return retv::Timeout;
    }
    retv SwitchCkSys2PLL() {
        SET_BITS(CFG0, 0b11UL, 0b10UL, 0); // Select PLL
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(BITS_EQUAL(CFG0, 0b11UL, 0b10UL, 2)) return retv::Ok;
        return retv::Timeout;
    }
    retv SwitchCkSys2XTAL() {
        SET_BITS(CFG0, 0b11UL, 0b01UL, 0); // Select XTAL
        for(uint32_t i=0; i<CLK_STARTUP_TIMEOUT; i++)
            if(BITS_EQUAL(CFG0, 0b11UL, 0b01UL, 2)) return retv::Ok;
        return retv::Timeout;
    }

    void SetAhbPrescaler(AhbPsc Div) { SET_BITS(CFG0, 0b1111UL, (uint32_t)Div, 4); }

    // USB: the USBFS clock must be 48MHz. Do not touch if USBFS clk is enabled.
    void SetUsbPrescaler(UsbPsc Div) { CFG0 = (CFG0 & ~((1UL << 31) | (0b11UL << 22))) | (uint32_t)Div; }
    void SetCK48MSel_CKPLL()    { ADDCTL &= ~RCU_ADDCTL_CK48MSEL; }
    void SetCK48MSel_CKIRC48M() { ADDCTL |=  RCU_ADDCTL_CK48MSEL; }
#endif

#if 1 // ==== Enable periperial blocks ====
    void EnDMAs() { AHBEN |= (1UL << 0) | (1UL << 1); }
    void EnUSB()  { AHBEN |= 1UL << 12; }
    void EnCRC()  { AHBEN |= 1UL << 6;  }

    void EnAFIO() { APB2EN |= 1UL <<  0; }
    void EnADC0() { APB2EN |= 1UL <<  9; }
    void EnADC1() { APB2EN |= 1UL << 10; }

    void EnI2C0() { APB1EN |= 1UL << 21; }
    void EnI2C1() { APB1EN |= 1UL << 22; }
    void EnBckp() { APB1EN |= 1UL << 27; }
    void EnPwrMgmtUnit() { APB1EN |= 1UL << 28; }
    void EnDAC()  { APB1EN |= 1UL << 29; }

    void EnCTC()  { ADDAPB1EN |= 1UL << 27; }

    void EnUart(const USART_TypeDef* PUart) {
        if     (PUart == USART0) APB2EN |= 1UL << 14;
        else if(PUart == USART1) APB1EN |= 1UL << 17;
        else if(PUart == USART2) APB1EN |= 1UL << 18;
        else if(PUart == UART3)  APB1EN |= 1UL << 19;
        else if(PUart == UART4)  APB1EN |= 1UL << 20;
    }

    void EnGpio(const GPIO_TypeDef *pgpio) {
        if     (pgpio == GPIOA) APB2EN |= 1UL << 2;
        else if(pgpio == GPIOB) APB2EN |= 1UL << 3;
        else if(pgpio == GPIOC) APB2EN |= 1UL << 4;
        else if(pgpio == GPIOD) APB2EN |= 1UL << 5;
        else if(pgpio == GPIOE) APB2EN |= 1UL << 6;
    }

    void EnTimer(const TIM_TypeDef *PTimer) {
        if     (PTimer == TIM0)  APB2EN |= 1UL << 11;
        else if(PTimer == TIM1)  APB1EN |= 1UL <<  0;
        else if(PTimer == TIM2)  APB1EN |= 1UL <<  1;
        else if(PTimer == TIM3)  APB1EN |= 1UL <<  2;
        else if(PTimer == TIM4)  APB1EN |= 1UL <<  3;
        else if(PTimer == TIM5)  APB1EN |= 1UL <<  4;
        else if(PTimer == TIM6)  APB1EN |= 1UL <<  5;
        else if(PTimer == TIM7)  APB2EN |= 1UL << 13;
        else if(PTimer == TIM8)  APB2EN |= 1UL << 19;
        else if(PTimer == TIM9)  APB2EN |= 1UL << 20;
        else if(PTimer == TIM10) APB2EN |= 1UL << 21;
        else if(PTimer == TIM11) APB1EN |= 1UL <<  6;
        else if(PTimer == TIM12) APB1EN |= 1UL <<  7;
        else if(PTimer == TIM13) APB1EN |= 1UL <<  8;
    }

    void EnSpi(const SPI_TypeDef *PSpi) {
        if     (PSpi == SPI0) APB2EN |= 1UL << 12;
        else if(PSpi == SPI1) APB1EN |= 1UL << 14;
        else if(PSpi == SPI2) APB1EN |= 1UL << 15;
    }
#endif

#if 1 // ==== Disable periperial blocks ====
    void DisAllPeriph() {
        AHBEN = 0;
        APB1EN = 0;
        APB2EN = 0;
        ADDAPB1EN = 0;
    }

    void DisUSB()  { AHBEN  &= ~(1UL << 12); }
    void DisI2C0() { APB1EN &= ~(1UL << 21); }
    void DisI2C1() { APB1EN &= ~(1UL << 22); }

    void DisTimer(const TIM_TypeDef *PTimer) {
        if     (PTimer == TIM0)  APB2EN &= ~(1UL << 11);
        else if(PTimer == TIM1)  APB1EN &= ~(1UL <<  0);
        else if(PTimer == TIM2)  APB1EN &= ~(1UL <<  1);
        else if(PTimer == TIM3)  APB1EN &= ~(1UL <<  2);
        else if(PTimer == TIM4)  APB1EN &= ~(1UL <<  3);
        else if(PTimer == TIM5)  APB1EN &= ~(1UL <<  4);
        else if(PTimer == TIM6)  APB1EN &= ~(1UL <<  5);
        else if(PTimer == TIM7)  APB2EN &= ~(1UL << 13);
        else if(PTimer == TIM8)  APB2EN &= ~(1UL << 19);
        else if(PTimer == TIM9)  APB2EN &= ~(1UL << 20);
        else if(PTimer == TIM10) APB2EN &= ~(1UL << 21);
        else if(PTimer == TIM11) APB1EN &= ~(1UL <<  6);
        else if(PTimer == TIM12) APB1EN &= ~(1UL <<  7);
        else if(PTimer == TIM13) APB1EN &= ~(1UL <<  8);
    }
#endif

#if 1 // ==== Reset ====
    void ResetAll() {
        AHBRST = 0xFFFFFFFF;
        AHBRST = 0x00000000;
        (void)AHBRST; // Perform dummy read
        APB1RST = 0xFFFFFFFF;
        APB1RST = 0x00000000;
        (void)APB1RST;
        APB2RST = 0xFFFFFFFF;
        APB2RST = 0x00000000;
        (void)APB2RST;
        ADDAPB1RST = 0xFFFFFFFF;
        ADDAPB1RST = 0x00000000;
        (void)ADDAPB1RST;
    }

    void ResI2C0() {
        APB1RST |=  (1UL << 21);
        APB1RST &= ~(1UL << 21);
        (void)APB1RST;
    }
    void ResI2C1() {
        APB1RST |=  (1UL << 22);
        APB1RST &= ~(1UL << 22);
        (void)APB1RST;
    }

    void ResUSB() {
        AHBRST |=  (1UL << 12);
        AHBRST &= ~(1UL << 12);
        (void)AHBRST; // Perform dummy read
    }

    void ResCTC()  {
        ADDAPB1RST = 1UL << 27;
        ADDAPB1RST = 0;
        (void)ADDAPB1RST; // Perform dummy read
    }
#endif
    // ADC clock must be within [0.1; 40] MHz
    void SetAdcPsc(AdcPsc Psc) {
        uint32_t w = (uint32_t)Psc;
        // Put Psc[2] to bit 28, Psc[1:0] to bits [15:14] of CFG0
        uint32_t tmp = CFG0 & ~((1UL << 28) | (1UL << 15) | (1UL << 14));
        tmp |= (w & 0b0011UL) << 14;
        if(w & 0b0100UL) tmp |= (1UL << 28);
        CFG0 = tmp;
        // Put Psc[3] to bit 29 of CFG1
        if(w & 0b1000UL) CFG1 |= (1UL << 29);
        else CFG1 &= ~(1UL << 29);
    }

#if 1 // ==== Get ====
    uint32_t GetCkSys() {
        uint32_t CkSysSrc = GET_BITS(CFG0, 0b11UL, 2);
        if(CkSysSrc == 0b01UL) return CRYSTAL_FREQ_HZ; // Ext crystal
        else if(CkSysSrc == 0b10UL) { // PLL
            uint32_t PllSrcFreqHz;
            if((CFG0 & RCU_CFG0_PLLSEL) == 0) PllSrcFreqHz = IRC8M_Hz / 2UL;
            else { // Prediv0 is src
                // Get Prediv0InputFreq
                uint32_t Prediv0InputFreq;
                if((CFG1 & RCU_CFG1_PREDV0SEL) == 0) {
                    if(CFG1 & RCU_CFG1_PLLPRESEL) Prediv0InputFreq = IRC48M_Hz; // PLLPRESEL == 1
                    else Prediv0InputFreq = CRYSTAL_FREQ_HZ; // PLLPRESEL == 0
                }
                else { // Predv0Sel == 1 => Prediv0InputFreq is output of PLL1
                    // Get PLL1 multi
                    uint32_t Pll1Mf = GET_BITS(CFG1, 0b1111UL, 8);
                    uint32_t Pll1Multi = 0;
                    if(Pll1Mf >= 0b0110UL) Pll1Multi = Pll1Mf + 2UL; // lower values are reserved
                    if(Pll1Mf == 0b1101UL) Pll1Multi = 0;   // 1101 is reserved
                    if(Pll1Mf == 0b1111UL) Pll1Multi = 20;  // 1111 is special case
                    // Get PLL1 div
                    uint32_t Prediv1 = GET_BITS(CFG1, 0b1111UL, 4) + 1UL;
                    // Get PLL1 src:
                    uint32_t Prediv1InputFreq = (CFG1 & RCU_CFG1_PLLPRESEL)? IRC48M_Hz : CRYSTAL_FREQ_HZ;
                    // Calculate Prediv0InputFreq
                    Prediv0InputFreq = Pll1Multi * (Prediv1InputFreq / Prediv1);
                }
                // Calculate PllSrcFreqHz
                uint32_t Prediv0 = GET_BITS(CFG1, 0b1111UL, 0) + 1UL;
                PllSrcFreqHz = Prediv0InputFreq / Prediv0;
            }
            // Calculate CkSysHz
            uint32_t PllMf = GET_BITS(CFG0, 0b1111UL, 18) | (GET_BITS(CFG0, 1UL, 29) << 4);
            if     (PllMf <= 0b01100UL) return PllSrcFreqHz * (PllMf + 2UL); // Multi = [2; 14]
            else if(PllMf == 0b01101UL) return (PllSrcFreqHz * 13UL) / 2UL;  // Multi = 6.5
            else if(PllMf == 0b01110UL) return PllSrcFreqHz * 16UL;          // Multi = 16
            else if(PllMf == 0b11111UL) return PllSrcFreqHz * 31UL;          // Multi = 31
            else                        return PllSrcFreqHz * (PllMf + 1UL); // PllMf == [01111; 11110] => Multi = [16; 31]
        }
        else return IRC8M_Hz; // for 00 and 11
    }
#endif // Get
};

#define RCU     ((RCU_TypeDef*)RCU_BASE)
#endif // RCU

#if 1 // ================== Clock trim controller (CTC) ========================
// For detailed description, see User Manual CTC Software program guide, p.108
#define CTC_FCLOCK_HZ           48000000UL // 48MHz of IRC48M
#define CTC_RVALUE(Fref_Hz)     ((CTC_FCLOCK_HZ / Fref_Hz) - 1UL)
#define CTC_CKLIM(Fref_Hz)      ((((CTC_FCLOCK_HZ / Fref_Hz) * 12UL) / 10000UL) / 2UL)

#define CTC_CTL0_AUTOTRIM       (1UL << 6)
#define CTC_CTL0_CNTEN          (1UL << 5)

#define CTC_REFPOL_RISING       (0UL << 31)
#define CTC_REFPOL_FALLING      (1UL << 31)
#define CTC_REFSEL_GPIO         (0b00UL << 28)
#define CTC_REFSEL_LXTAL        (0b01UL << 28)
#define CTC_REFSEL_USBSOF       (0b10UL << 28)

#define CTC_REFPSC_DIV_1        (0b000UL << 24)
#define CTC_REFPSC_DIV_2        (0b001UL << 24)
#define CTC_REFPSC_DIV_4        (0b010UL << 24)
#define CTC_REFPSC_DIV_8        (0b011UL << 24)
#define CTC_REFPSC_DIV_16       (0b100UL << 24)
#define CTC_REFPSC_DIV_32       (0b101UL << 24)
#define CTC_REFPSC_DIV_64       (0b110UL << 24)
#define CTC_REFPSC_DIV_128      (0b111UL << 24)

struct CTC_Typedef {
    volatile uint32_t CTL0;  /*!< 0x00 Control register 0 */
    volatile uint32_t CTL1;  /*!< 0x04 Control register 1 */
    volatile uint32_t STAT;  /*!< 0x08 Status register */
    volatile uint32_t INTC;  /*!< 0x0C Interrupt clear register */

    void Setup(uint32_t RefPol, uint32_t RefSel, uint32_t RefPsc, uint32_t Fref) {
        CTL1 = RefPol | RefSel | RefPsc | (CTC_CKLIM(Fref) << 16) | CTC_RVALUE(Fref);
        CTL0 |= CTC_CTL0_AUTOTRIM; // Enable autotrim. Otherwise, what's the point?
    }

    void Enable()  { CTL0 |=  CTC_CTL0_CNTEN; }
    void Disable() { CTL0 &= ~CTC_CTL0_CNTEN; }
};

#define CTC     ((CTC_Typedef*)CTC_BASE)
#endif

#if 1 // ============================= Flash ===================================
#define FMC_WS_PGW          (1UL << 15) // Program width to flash memory: 0 is 32bit (default), 1 is 64bit
#define FMC_WS_DCRST        (1UL << 12) // DBUS cache reset. This bit can be write only when DCEN is set to 0
#define FMC_WS_ICRST        (1UL << 11) // IBUS cache reset. This bit can be write only when ICEN is set to 0
#define FMC_WS_DCEN         (1UL << 10) // DBUS cache enable
#define FMC_WS_ICEN         (1UL <<  9) // IBUS cache enable
#define FMC_WS_PFEN         (1UL <<  4) // Pre-fetch enable
#define FMC_WS_WSCNT_MSK    (7UL <<  0) // Wait state mask

#define FMC_STAT_ENDF       (1UL << 5) // End of operation flag
#define FMC_STAT_WPERR      (1UL << 4) // Erase/Program protection error flag
#define FMC_STAT_PGAERR     (1UL << 3) // Program alignment error flag
#define FMC_STAT_PGERR      (1UL << 2) // Program error flag
#define FMC_STAT_BUSY       (1UL << 0) // The flash is busy

#define FMC_CTL_OBWEN       (1UL << 9) // Option byte erase/program enable bit
#define FMC_CTL_LK          (1UL << 7) // FMC_CTL lock bit
#define FMC_CTL_START       (1UL << 6) // Send erase command to FMC bit
#define FMC_CTL_OBER        (1UL << 5) // Option bytes erase command bit
#define FMC_CTL_OBPG        (1UL << 4) // Option bytes program command bit
#define FMC_CTL_MER         (1UL << 2) // Main flash mass erase for bank0 command bit
#define FMC_CTL_PER         (1UL << 1) // Main flash page erase for bank0 command bit
#define FMC_CTL_PG          (1UL << 0) // Main flash program for bank0 command bit

struct FMC_TypeDef {
    volatile uint32_t WS;     /*!< 0x00  Wait state register */
    volatile uint32_t KEY;    /*!< 0x04  Unlock key register */
    volatile uint32_t OBKEY;  /*!< 0x08  Option byte unlock key register */
    volatile uint32_t STAT;   /*!< 0x0C  Status register */
    volatile uint32_t CTL;    /*!< 0x10 Control register */
    volatile uint32_t ADDR;   /*!< 0x14 Address register */
    volatile uint32_t _Reserved1; // 18
    volatile uint32_t OBSTAT; /*!< 0x1C Option byte status register */
    volatile uint32_t WP;     /*!< 0x20  Erase/Program protection register */
    volatile uint32_t _Reserved[55];
    volatile uint32_t PID;     /*!< 0x100 Product ID register */

    void EnableCashAndPrefetch() { WS |= (1UL<<10)|(1UL<<9)|(1UL<<4); }

    void SetLatency(uint32_t Clk_MHz) {
        uint32_t tmp = WS & ~FMC_WS_WSCNT_MSK; // Clean last 3 bits
        if     (Clk_MHz <=  30) tmp |= 0UL; // Add 0 wait state
        else if(Clk_MHz <=  60) tmp |= 1UL; // Add 1 wait state
        else if(Clk_MHz <=  90) tmp |= 2UL; // Add 2 wait state
        else if(Clk_MHz <= 120) tmp |= 3UL; // Add 3 wait state
        WS = tmp;
    }
};

#define FMC     ((FMC_TypeDef*)FMC_BASE)
#endif

#if 1 // ======================= Option Bytes ==================================
struct OPTB_Typedef {
    union {
        volatile uint32_t SPC_USER32;
        struct {
            volatile uint8_t SPC;   // Security Protection value
            volatile uint8_t SPC_N; // SPC complement value
            volatile uint8_t USER;  // nRST_STDBY, nRST_DPSLP, nWDG_HW
            volatile uint8_t USER_N;
        };
    };
};

#define OPTBYTES    ((OPTB_Typedef*)OB_BASE)
#endif

#if 1 // ============================= USB =====================================
// Resources
#define USBFS_MAX_TX_FIFOS      15U         /*!< FIFO number */
//#define USBFS_MAX_PACKET_SIZE   64U
#define USBFS_MAX_CHANNEL_COUNT 8U          /*!< USBFS host channel count */
#define USBFS_MAX_EP_COUNT      4U          /*!< USBFS device endpoint count, including EP0 */
#define USBFS_MAX_FIFO_WORDLEN  320U        /*!< USBFS max fifo size in words */

#define USB_DATA_FIFO_OFFSET    0x1000U
#define USB_DATA_FIFO_SIZE      0x1000U

// Bits
#define GUSBCS_FDM              (1U<<30)    // Force Device Mode
#define GUSBCS_FHM              (1U<<29)    // Force Host Mode
#define GUSBCS_UTT(n)           ((n)<<10)   // USB Turnaround time field value
#define GUSBCS_UTT_MASK         (0xFUL<<10)
#define GUSBCS_EMBPHY           (1UL<<6) // Not used in E113, but who knows

#define GRSTCTL_CSRST           (1UL<<0)
#define GRSTCTL_RXFF            (1U<<4)     /**< RxFIFO flush.              */
#define GRSTCTL_TXFF            (1U<<5)     /**< TxFIFO flush.              */
#define GRSTCTL_TXFNUM(n)       ((n)<<6)    /**< TxFIFO number field value. */

#define GAHBCS_GINTEN           (1U<< 0) // Global interrupt mask

#define GINTF_SOF               (1UL<< 3) // Start of frame
#define GINTF_RXFNEIF           (1UL<< 4) // Rx FIFO non-empty interrupt flag
#define GINTF_SP                (1UL<<11) // USB suspend
#define GINTF_RST               (1UL<<12) // USB reset
#define GINTF_ENUMF             (1UL<<13) // Enumeration finished
#define GINTF_IEPIF             (1UL<<18) // IN endpoint interrupt flag
#define GINTF_OEPIF             (1UL<<19) // OUT endpoint interrupt flag
#define GINTF_ISOINCIF          (1UL<<20) // Isochronous IN transfer not complete interrupt flag
#define GINTF_ISOONCIF          (1UL<<21) // Isochronous OUT transfer not complete interrupt flag
#define GINTF_WKUPIF            (1UL<<31) // Wakeup interrupt flag

#define GINTEN_MFIE             (1UL<< 1) // Mode fault interrupt enable
#define GINTEN_OTGIE            (1UL<< 2) // OTG interrupt enable
#define GINTEN_SOFIE            (1UL<< 3) // Start of frame interrupt enable
#define GINTEN_RXFNEIE          (1UL<< 4) // Receive FIFO non-empty interrupt enable
#define GINTEN_NPTXFEIE         (1UL<< 5) // Non-periodic Tx FIFO empty interrupt enable
#define GINTEN_GNPINAKIE        (1UL<< 6) // Global non-periodic IN NAK effective interrupt enable
#define GINTEN_GONAKIE          (1UL<< 7) // Global OUT NAK effective interrupt enable
#define GINTEN_ESPIE            (1UL<<10) // Early suspend interrupt enable
#define GINTEN_SPIE             (1UL<<11) // USB suspend interrupt enable
#define GINTEN_RSTIE            (1UL<<12) // USB reset interrupt enable
#define GINTEN_ENUMFIE          (1UL<<13) // Enumeration finish enable
#define GINTEN_ISOOPDIE         (1UL<<14) // Isochronous OUT packet dropped interrupt enable
#define GINTEN_EOPFIE           (1UL<<15) // End of periodic frame interrupt enable
#define GINTEN_IEPIE            (1UL<<18) // IN endpoints interrupt enable
#define GINTEN_OEPIE            (1UL<<19) // OUT endpoints interrupt enable
#define GINTEN_ISOINCIE         (1UL<<20) // Isochronous IN transfer not complete interrupt enable
#define GINTEN_ISOONCIE         (1UL<<21) // Isochronous OUT transfer not complete interrupt enable
#define GINTEN_PXNCIE           (1UL<<21) // Yes, 21. Periodic transfer not complete Interrupt enable
#define GINTEN_HPIE             (1UL<<24) // Host port interrupt enable
#define GINTEN_HCIE             (1UL<<25) // Host channels interrupt enable
#define GINTEN_PTXFEIE          (1UL<<26) // Periodic Tx FIFO empty interrupt enable
#define GINTEN_IDPSCIE          (1UL<<28) // ID pin status change interrupt enable
#define GINTEN_DISCIE           (1UL<<29) // Disconnected interrupt enable
#define GINTEN_SESIE            (1UL<<30) // Session interrupt enable
#define GINTEN_WKUPIE           (1UL<<31) // Wakeup interrupt enable

#define GRSTATP_PKTSTS_MASK     (0xFUL<<17) // Packet status mask
#define GRSTATP_PKTSTS(n)       ((n)<<17)   // Packet status value
#define GRSTATP_OUT_GLOBAL_NAK  GRSTATP_PKTSTS(1)
#define GRSTATP_OUT_DATA        GRSTATP_PKTSTS(2)
#define GRSTATP_OUT_COMP        GRSTATP_PKTSTS(3)
#define GRSTATP_SETUP_COMP      GRSTATP_PKTSTS(4)
#define GRSTATP_SETUP_DATA      GRSTATP_PKTSTS(6)
#define GRSTATP_BCNT_MASK       (0x7FFUL<<4) // Byte count mask
#define GRSTATP_BCNT_OFFSET     4           // Byte count offset
#define GRSTATP_BCNT(n)         ((n)<<4)    // Byte count value
#define GRSTATP_EPNUM_MASK      0xFUL       // Endpoint number mask
#define GRSTATP_EPNUM_OFFSET    0           // Endpoint number offset
#define GRSTATP_EPNUM(n)        ((n)<<0)    // Endpoint number value

#define DIEPTFLEN_IEPTXFD_MASK   (0xFFFFU<<16) // IN endpoint TxFIFO depth mask
#define DIEPTFLEN_IEPTXFD(n)     ((n)<<16)   // IN endpoint TxFIFO depth value
#define DIEPTFLEN_IEPTXRSAR_MASK (0xFFFF<<0) // IN endpoint FIFOx transmit RAM start address mask
#define DIEPTFLEN_IEPTXRSAR(n)   ((n)<<0)    // IN endpoint FIFOx transmit RAM start address value

#define GCCFG_VBUSIG            (1UL<<21)   // VBUS ignored
#define GCCFG_SOFOEN            (1UL<<20)   // SOF output enable
#define GCCFG_VBUSBCEN          (1UL<<19)   // The VBUS B-Device comparer enable
#define GCCFG_VBUSACEN          (1UL<<18)   // The VBUS A-Device comparer enable
#define GCCFG_PWRON             (1UL<<16)

#define DCFG_DSPD_FS11          (3U<<0)     /**< Full speed (USB 1.1 transceiver clock is 48MHz) */
#define DCFG_DAD_MASK           (0x7FU<<4)  /**< Device address mask.       */
#define DCFG_DAD(n)             ((n)<<4)    /**< Device address value.      */

#define DCTL_RWKUP              (1U<<0)     // Remote wakeup signaling
#define DCTL_SD                 (1U<<1)     // Soft disconnect

#define DSTS_SUSPSTS            (1U<<0)     /**< Suspend status.            */
#define DSTS_ENUMSPD_MASK       (3U<<1)     /**< Enumerated speed mask.     */
#define DSTS_ENUMSPD_HS_480     (0U<<1)     /**< High speed.                */
#define DSTS_ENUMSPD_FS_48      (3U<<1)     /**< Full speed (PHY clock is running at 48 MHz).        */
#define DSTS_EERR               (1U<<3)     /**< Erratic error.             */
#define DSTS_FNSOF_MASK         (0x3FFU<<8) // Frame number of the received SOF mask
#define DSTS_FNSOF(n)           ((n)<<8)    // Frame number of the received SOF value
#define DSTS_FNSOF_ODD          (1U<<8)     // Frame parity of the received SOF value

#define DIEPMSK_TXFEM           (1U<<6)     /**< Transmit FIFO empty mask.  */
#define DIEPMSK_INEPNEM         (1U<<6)     // IN endpoint NAK effective mask
#define DIEPMSK_ITTXFEMSK       (1U<<4)     // IN token received when TxFIFO empty mask
#define DIEPMSK_TOCM            (1U<<3)     /**< Timeout condition mask.    */
#define DIEPMSK_EPDM            (1U<<1)     // Endpoint disabled interrupt mask
#define DIEPMSK_XFRCM           (1U<<0)     // Transfer completed interrupt mask

#define DOEPMSK_OTEPDM          (1U<<4)     // OUT token received when endpoint disabled mask
#define DOEPMSK_STUPM           (1U<<3)     // SETUP phase done mask
#define DOEPMSK_EPDM            (1U<<1)     // Endpoint disabled interrupt mask
#define DOEPMSK_XFRCM           (1U<<0)     // Transfer completed interrupt mask

#define DAINTMSK_OEPM(n)        (1U<<(16+(n))) /**< OUT EP interrupt mask bits value */
#define DAINTMSK_IEPM(n)        (1U<<(n))   // IN EP interrupt mask bits value

#define DIEPEMPMSK_INEPTXFEM(n) (1U<<(n))   // IN EP Tx FIFO empty interrupt mask bit

#define DTXFSTS_INEPTFSAV_MASK  0xFFFFUL    // IN endpoint TxFIFO space available

#define DIEPCTL_EPENA           (1U<<31)    /**< Endpoint enable.           */
#define DIEPCTL_EPDIS           (1U<<30)    /**< Endpoint disable.          */
#define DIEPCTL_SD1PID          (1U<<29)    /**< Set DATA1 PID.             */
#define DIEPCTL_SODDFRM         (1U<<29)    /**< Set odd frame.             */
#define DIEPCTL_SD0PID          (1U<<28)    /**< Set DATA0 PID.             */
#define DIEPCTL_SEVNFRM         (1U<<28)    /**< Set even frame.            */
#define DIEPCTL_SNAK            (1U<<27)    /**< Set NAK.                   */
#define DIEPCTL_CNAK            (1U<<26)    /**< Clear NAK.                 */
#define DIEPCTL_TXFNUM_MASK     (15U<<22)   /**< TxFIFO number mask.        */
#define DIEPCTL_TXFNUM(n)       ((n)<<22)   /**< TxFIFO number value.       */
#define DIEPCTL_STALL           (1U<<21)    /**< STALL handshake.           */
#define DIEPCTL_SNPM            (1U<<20)    /**< Snoop mode.                */
#define DIEPCTL_EPTYP_MASK      (3<<18)     /**< Endpoint type mask.        */
#define DIEPCTL_EPTYP_CTRL      (0U<<18)    /**< Control.                   */
#define DIEPCTL_EPTYP_ISO       (1U<<18)    /**< Isochronous.               */
#define DIEPCTL_EPTYP_BULK      (2U<<18)    /**< Bulk.                      */
#define DIEPCTL_EPTYP_INTR      (3U<<18)    /**< Interrupt.                 */
#define DIEPCTL_NAKSTS          (1U<<17)    /**< NAK status.                */
#define DIEPCTL_EONUM           (1U<<16)    /**< Even/odd frame.            */
#define DIEPCTL_DPID            (1U<<16)    /**< Endpoint data PID.         */
#define DIEPCTL_USBAEP          (1U<<15)    /**< USB active endpoint.       */
#define DIEPCTL_MPSIZ_MASK      (0x3FFU<<0) /**< Maximum Packet size mask.  */
#define DIEPCTL_MPSIZ(n)        ((n)<<0)    /**< Maximum Packet size value. */

#define DOEPCTL_EPENA           (1U<<31)    /**< Endpoint enable.           */
#define DOEPCTL_EPDIS           (1U<<30)    /**< Endpoint disable.          */
#define DOEPCTL_SD1PID          (1U<<29)    /**< Set DATA1 PID.             */
#define DOEPCTL_SODDFRM         (1U<<29)    /**< Set odd frame.             */
#define DOEPCTL_SD0PID          (1U<<28)    /**< Set DATA0 PID.             */
#define DOEPCTL_SEVNFRM         (1U<<28)    /**< Set even frame.            */
#define DOEPCTL_SNAK            (1U<<27)    /**< Set NAK.                   */
#define DOEPCTL_CNAK            (1U<<26)    /**< Clear NAK.                 */
#define DOEPCTL_STALL           (1U<<21)    /**< STALL handshake.           */
#define DOEPCTL_SNPM            (1U<<20)    /**< Snoop mode.                */
#define DOEPCTL_EPTYP_MASK      (3U<<18)    /**< Endpoint type mask.        */
#define DOEPCTL_EPTYP_CTRL      (0U<<18)    /**< Control.                   */
#define DOEPCTL_EPTYP_ISO       (1U<<18)    /**< Isochronous.               */
#define DOEPCTL_EPTYP_BULK      (2U<<18)    /**< Bulk.                      */
#define DOEPCTL_EPTYP_INTR      (3U<<18)    /**< Interrupt.                 */
#define DOEPCTL_NAKSTS          (1U<<17)    /**< NAK status.                */
#define DOEPCTL_EONUM           (1U<<16)    /**< Even/odd frame.            */
#define DOEPCTL_DPID            (1U<<16)    /**< Endpoint data PID.         */
#define DOEPCTL_USBAEP          (1U<<15)    /**< USB active endpoint.       */
#define DOEPCTL_MPSIZ_MASK      (0x3FFU<<0) /**< Maximum Packet size mask.  */
#define DOEPCTL_MPSIZ(n)        ((n)<<0)    /**< Maximum Packet size value. */

#define DIEPINT_TXFE            (1U<<7)     // Transmit FIFO empty
#define DIEPINT_INEPNE          (1U<<6)     // IN endpoint NAK effective
#define DIEPINT_ITTXFE          (1U<<4)     // IN Token received when TxFIFO is empty
#define DIEPINT_TOC             (1U<<3)     // Timeout condition
#define DIEPINT_EPDISD          (1U<<1)     // Endpoint disabled interrupt
#define DIEPINT_XFRC            (1U<<0)     // Transfer completed

#define DOEPINT_SETUP_RCVD      (1U<<15)    // SETUP packet received
#define DOEPINT_B2BSTUP         (1U<<6)     // Back-to-back SETUP packets received
#define DOEPINT_OTEPDIS         (1U<<4)     // OUT token received when endpoint disabled
#define DOEPINT_STUP            (1U<<3)     // SETUP phase done
#define DOEPINT_EPDISD          (1U<<1)     // Endpoint disabled  interrupt
#define DOEPINT_XFRC            (1U<<0)     // Transfer completed interrupt

#define DOEPTSIZ_RXDPID_MASK    (3U<<29)    /**< Received data PID mask.    */
#define DOEPTSIZ_RXDPID(n)      ((n)<<29)   /**< Received data PID value.   */
#define DOEPTSIZ_STUPCNT_MASK   (3U<<29)    /**< SETUP packet count mask.   */
#define DOEPTSIZ_STUPCNT(n)     ((n)<<29)   /**< SETUP packet count value.  */
#define DOEPTSIZ_PKTCNT_MASK    (0x3FFU<<19)/**< Packet count mask.         */
#define DOEPTSIZ_PKTCNT(n)      ((n)<<19)   /**< Packet count value.        */
#define DOEPTSIZ_XFRSIZ_MASK    (0x7FFFFU<<0)/**< Transfer size mask.       */
#define DOEPTSIZ_XFRSIZ(n)      ((n)<<0)    /**< Transfer size value.       */

#define DIEPTSIZ_MCNT_MASK      (3U<<29)    /**< Multi count mask.          */
#define DIEPTSIZ_MCNT(n)        ((n)<<29)   /**< Multi count value.         */
#define DIEPTSIZ_PKTCNT_MASK    (0x3FF<<19) /**< Packet count mask.         */
#define DIEPTSIZ_PKTCNT(n)      ((n)<<19)   /**< Packet count value.        */
#define DIEPTSIZ_XFRSIZ_MASK    (0x7FFFFU<<0)/**< Transfer size mask.       */
#define DIEPTSIZ_XFRSIZ(n)      ((n)<<0)    /**< Transfer size value.       */

#define PWRCLKCTL_SHCLK         (1UL<<1) // Stop HCLK
#define PWRCLKCTL_SUCLK         (1UL<<0) // Stop SUCLK

// Host channel registers group.
struct UsbHostChnl_TypeDef {
    volatile uint32_t HCCHAR;     /**< @brief Host channel characteristics register */
    volatile uint32_t resvd8;
    volatile uint32_t HCINT;      /**< @brief Host channel interrupt register.*/
    volatile uint32_t HCINTMSK;   /**< @brief Host channel interrupt mask register */
    volatile uint32_t HCTSIZ;     /**< @brief Host channel transfer size register */
    volatile uint32_t resvd14;
    volatile uint32_t resvd18;
    volatile uint32_t resvd1c;
};

// Device input endpoint registers group.
struct UsbInEp_Typedef {
    volatile uint32_t DIEPCTL;    /**< @brief Device control IN endpoint control register */
    volatile uint32_t resvd4;
    volatile uint32_t DIEPINT;    /**< @brief Device IN endpoint interrupt register */
    volatile uint32_t resvdC;
    volatile uint32_t DIEPLEN;   /**< @brief Device IN endpoint transfer size register */
    volatile uint32_t resvd14;
    volatile uint32_t DIEPTFSTAT;    /**< @brief Device IN endpoint transmit FIFO status register */
    volatile uint32_t resvd1C;
};

// Device output endpoint registers group.
struct UsbOutEp_Typedef {
    volatile uint32_t DOEPCTL;    /**< @brief Device control OUT endpoint control register */
    volatile uint32_t resvd4;
    volatile uint32_t DOEPINTF;    /**< @brief Device OUT endpoint interrupt register */
    volatile uint32_t resvdC;
    volatile uint32_t DOEPTSIZ;   /**< @brief Device OUT endpoint transfer size register */
    volatile uint32_t resvd14;
    volatile uint32_t resvd18;
    volatile uint32_t resvd1C;
};

struct USB_Typedef {
    volatile uint32_t GOTGCS;    /**< @brief OTG control and status register.*/
    volatile uint32_t GOTGINTF;    /**< @brief OTG interrupt register.         */
    volatile uint32_t GAHBCS;     /**< @brief AHB configuration register.     */
    volatile uint32_t GUSBCS;    /**< @brief USB configuration register.     */
    volatile uint32_t GRSTCTL;    /**< @brief Reset register size.            */
    volatile uint32_t GINTF;    /**< @brief Interrupt register.             */
    volatile uint32_t GINTEN;    /**< @brief Interrupt mask register.        */
    volatile uint32_t GRSTATR;    /**< @brief Receive status debug read register */
    volatile uint32_t GRSTATP;    /**< @brief Receive status read/pop register */
    volatile uint32_t GRFLEN;    /**< @brief Receive FIFO size register.     */
    volatile uint32_t DIEP0TFLEN;   /**< @brief Endpoint 0 transmit FIFO size register */
    volatile uint32_t HNPTFQSTAT;   /**< @brief Non-periodic transmit FIFO/queue status register */
    volatile uint32_t resvd30;
    volatile uint32_t resvd34;
    volatile uint32_t GCCFG;      /**< @brief General core configuration.     */
    volatile uint32_t CID;        /**< @brief 3C Core ID register.               */
    volatile uint32_t resvd58[48];
    volatile uint32_t HPTFLEN;    // 100 Host periodic transmit FIFO size register
    volatile uint32_t DIEPTFLEN[15]; // 104 Device IN endpoint transmit FIFO size registers DIEPTXF
    volatile uint32_t resvd140[176];
    volatile uint32_t HCFG;       /**< @brief 400 Host configuration register.    */
    volatile uint32_t HFIR;       /**< @brief Host frame interval register.   */
    volatile uint32_t HFNUM;      /**< @brief Host frame number/frame time Remaining register */
    volatile uint32_t resvd40C;
    volatile uint32_t HPTXSTS;    /**< @brief Host periodic transmit FIFO/queue status register */
    volatile uint32_t HAINT;      /**< @brief Host all channels interrupt register */
    volatile uint32_t HAINTMSK;   /**< @brief Host all channels interrupt mask register */
    volatile uint32_t resvd41C[9];
    volatile uint32_t HPRT;       /**< @brief Host port control and status register */
    volatile uint32_t resvd444[47];
    UsbHostChnl_TypeDef hc[16];  /**< @brief Host channels array.            */
    volatile uint32_t resvd700[64];
    volatile uint32_t DCFG;       /**< @brief Device configuration register.  */
    volatile uint32_t DCTL;       /**< @brief Device control register.        */
    volatile uint32_t DSTS;       /**< @brief Device status register.         */
    volatile uint32_t resvd80C;
    volatile uint32_t DIEPINTEN;  // 810 Device IN endpoint common interrupt mask register
    volatile uint32_t DOEPINTEN;  // 814 Device OUT endpoint common interrupt mask register
    volatile uint32_t DAEPINT;    // 818 Device all endpoints interrupt register
    volatile uint32_t DAEPINTEN;   // 81C Device all endpoints interrupt mask register
    volatile uint32_t resvd820;
    volatile uint32_t resvd824;
    volatile uint32_t DVBUSDIS;   // 828 Device VBUS discharge time register
    volatile uint32_t DVBUSPULSE; // 82C Device VBUS pulsing time register
    volatile uint32_t resvd830;
    volatile uint32_t DIEPFEINTEN; // 834 Device IN endpoint FIFO empty interrupt mask register
    volatile uint32_t resvd838;
    volatile uint32_t resvd83C;
    volatile uint32_t resvd840[16];
    volatile uint32_t resvd880[16];
    volatile uint32_t resvd8C0[16];
    UsbInEp_Typedef ie[16];     /**< @brief Input endpoints.                */
    UsbOutEp_Typedef oe[16];    /**< @brief Output endpoints.               */
    volatile uint32_t resvdD00[64];
    volatile uint32_t PWRCLKCTL;    /**< @brief Power and clock gating control register */
    volatile uint32_t resvdE04[127];
    volatile uint32_t FIFO[16][1024];


    inline void ConnectBus()    { DCTL &= ~DCTL_SD; }
    inline void DisconnectBus() { DCTL |=  DCTL_SD; }

    inline void DisableGlobalIRQs() { GAHBCS &= ~GAHBCS_GINTEN; }
    inline void EnableGlobalIRQs()  { GAHBCS |=  GAHBCS_GINTEN; }

    // Endpoints
    void EpStallOut(uint32_t ep) { oe[ep].DOEPCTL |= DOEPCTL_STALL;  }
    void EpStallIn(uint32_t ep)  { ie[ep].DIEPCTL |= DIEPCTL_STALL;  }
    void EpClearOut(uint32_t ep) { oe[ep].DOEPCTL &= ~DOEPCTL_STALL; }
    void EpClearIn (uint32_t ep) { ie[ep].DIEPCTL &= ~DIEPCTL_STALL; }
};

#define USB         ((USB_Typedef*)USBFS_BASE)
#endif

#if 1 // ============================= i2c =====================================
#define I2C_CTL0_START      (1UL << 8)
#define I2C_CTL0_STOP       (1UL << 9)
#define I2C_CTL0_ACKEN      (1UL << 10)

#define I2C_CTL1_ERRIE      (1UL << 8)
#define I2C_CTL1_EVIE       (1UL << 9)
#define I2C_CTL1_BUFIE      (1UL << 10)

#define I2C_STAT0_SBSEND    (1UL << 0)
#define I2C_STAT0_ADDSEND   (1UL << 1)
#define I2C_STAT0_BTC       (1UL << 2)
#define I2C_STAT0_RBNE      (1UL << 6)
#define I2C_STAT0_TBE       (1UL << 7)
#define I2C_STAT0_BERR      (1UL << 8)
#define I2C_STAT0_LOSTARB   (1UL << 9)
#define I2C_STAT0_AERR      (1UL << 10)
#define I2C_STAT0_OUERR     (1UL << 11)
#define I2C_STAT0_PECERR    (1UL << 12)

#define I2C_STAT1_MASTER    (1UL << 0)
#define I2C_STAT1_I2CBSY    (1UL << 1)

struct I2C_TypeDef {
    volatile uint32_t CTL0;   /*!< Control register 0,         offset: 0x00 */
    volatile uint32_t CTL1;   /*!< Control register 1,         offset: 0x04 */
    volatile uint32_t SADDR0; /*!< Slave address register 0,   offset: 0x08 */
    volatile uint32_t SADDR1; /*!< Slave address register 1,   offset: 0x0C */
    volatile uint32_t DATA;   /*!< Transfer buffer register,   offset: 0x10 */
    volatile uint32_t STAT0;  /*!< Transfer status register 0, offset: 0x14 */
    volatile uint32_t STAT1;  /*!< Transfer status register 1, offset: 0x18 */
    volatile uint32_t CKCFG;  /*!< Clock configure register,   offset: 0x1C */
    volatile uint32_t RT;     /*!< Rise time register,         offset: 0x20 */
    volatile uint32_t resvd1[23];
    volatile uint32_t SAMCFG; /*!< SAM control and status register, Address offset: 0x80 */
    volatile uint32_t resvd2[3];
    volatile uint32_t FMPCFG; /*!< Fast-mode-plus configure register, Address offset: 0x90 */

    void Enable()  { CTL0 |=  (1UL << 0); }
    void Disable() { CTL0 &= ~(1UL << 0); }

    // Data
    uint8_t GetData() { return DATA; }
    void SendAddrWithWrite(uint8_t Addr) { DATA = (uint8_t)(Addr << 1); }
    void SendAddrWithRead (uint8_t Addr) { DATA = ((uint8_t)(Addr<<1)) | 0x01; }
    void SendData(uint32_t b) { DATA = b & 0xFFUL; }

    // Clock
    void SetDutycycle2()    { CKCFG &= ~(1UL << 14); }
    void SetDutycycle16d9() { CKCFG |=  (1UL << 14); }
    void SetSpeedStandard() { CKCFG &= ~(1UL << 15); }
    void SetSpeedFast()     { CKCFG |=  (1UL << 15); }
    void EnFastPlus()       { FMPCFG =  (1UL << 0);  }
    void SetClkc(uint32_t clkc) { SET_BITS(CKCFG, 0xFFFUL, clkc, 0); }

    // Flags
    void ClearStatFlags() { STAT0 = 0; }
    void ClearAddrFlag() { (void)STAT0; (void)STAT1; }
    bool IsBusy() { return STAT1 & (1UL << 1); }
    bool IsRxNotEmpty() { return STAT0 & I2C_STAT0_RBNE; }
    bool IsStartSent()  { return STAT0 & I2C_STAT0_SBSEND; }
    bool IsNACK()       { return STAT0 & I2C_STAT0_AERR; }
    bool IsAddrSentAndACKed() { return STAT0 & I2C_STAT0_ADDSEND; }

    bool IsStartSentAndBusyMaster() {
        uint32_t Flag0 = STAT0;
        uint32_t Flag1 = STAT1;
        return (Flag0 & I2C_STAT0_SBSEND) and (Flag1 & (I2C_STAT1_MASTER | I2C_STAT1_I2CBSY));
    }

    // IRQs
    void EnBufferIRQ()   { CTL1 |= I2C_CTL1_BUFIE; }
    void DisBufferIRQ()  { CTL1 &= ~I2C_CTL1_BUFIE; }
    void EnEventIRQ()    { CTL1 |= I2C_CTL1_EVIE; }
    void DisEventIRQ()   { CTL1 &= ~I2C_CTL1_EVIE; }
    void EnErrorIRQ()    { CTL1 |= I2C_CTL1_ERRIE; }
    void EnAllIRQs()     { CTL1 |= I2C_CTL1_BUFIE | I2C_CTL1_EVIE | I2C_CTL1_ERRIE; }
    void DisAllIRQs()    { CTL1 &= ~(I2C_CTL1_BUFIE | I2C_CTL1_EVIE | I2C_CTL1_ERRIE); }

    // Actions
    void SendStart()     { CTL0 |= I2C_CTL0_START; }
    void SendStop()      { CTL0 |= I2C_CTL0_STOP; }
    void EnAck()         { CTL0 |= I2C_CTL0_ACKEN; }
    void DisAck()        { CTL0 &= ~I2C_CTL0_ACKEN; }
    void FlushDatabuf()  { while(IsRxNotEmpty()) (void)DATA; } // Read DR until it empty
};

#define I2C0    ((I2C_TypeDef*)I2C0_BASE)
#define I2C1    ((I2C_TypeDef*)I2C1_BASE)

#define I2C_DUTYCYCLE_2     1
#define I2C_DUTYCYCLE_16_9  2

#define I2CCLK_MIN_MHz      2UL
#define I2CCLK_MAX_MHz      60UL // > 60 is not allowed due to the limitation of APB1 clock
#endif

#if 1 // ============================= ADC =====================================
#define VREFINT_mV              1200UL // See datasheet, ADC description
// Inner ADC channels. See datasheet
#define ADC_CHNL_Temperature    16UL
#define ADC_CHNL_Vrefint        17UL

#define ADC_CTL0_SM         (1UL <<  8)

#define ADC_CTL1_ADCON      (1UL <<  0)
#define ADC_CTL1_CTN        (1UL <<  1)
#define ADC_CTL1_CLB        (1UL <<  2)
#define ADC_CTL1_RSTCLB     (1UL <<  3)
#define ADC_CTL1_DMA        (1UL <<  8)
#define ADC_CTL1_ETERC      (1UL << 20)
#define ADC_CTL1_SWRCST     (1UL << 22)
#define ADC_CTL1_TSVREN     (1UL << 23)

#define ADC_OVSAMPCTL_OVSEN (1UL << 0)

enum class AdcExtTrgSrc { Tim0C0=0b000UL, Tim0C1=0b001UL, Tim0C2=0b010UL, Tim1C1=0b011UL,
    Tim2TRGO=0b100UL, Tim3C3=0b101UL, Exti11_Tim7TRGO=0b110UL, Swrcst=0b111UL};

enum class AdcSampleTime {
    t1d5Cycles  = 0b000,
    t7d5Cycles  = 0b001,
    t13d5Cycles = 0b010,
    t28d5Cycles = 0b011,
    t41d5Cycles = 0b100,
    t55d5Cycles = 0b101,
    t71d5Cycles = 0b110,
    t239dCycles = 0b111
};

enum class AdcOversamplingRatio : uint32_t { Disabled=0xFFUL, x2=0b000UL, x4=0b001UL,
    x8=0b010UL, x16=0b011UL, x32=0b100UL, x64=0b101UL, x128=0b110UL, x256=0b111UL};
enum class AdcOversamplingShift : uint32_t { NoShift=0b0000UL, sh1bit=0b0001UL,
    sh2bits=0b0010UL, sh3bits=0b0011UL, sh4bits=0b0100UL, sh5bits=0b0101UL,
    sh6bits=0b0110UL, sh7bits=0b0111UL, sh8bits=0b1000UL};

struct ADC_TypeDef {
    volatile uint32_t STAT;      /*!< Status register,                  0x00 */
    volatile uint32_t CTL0;      /*!< Control register 0,               0x04 */
    volatile uint32_t CTL1;      /*!< Control register 1,               0x08 */
    volatile uint32_t SAMPT0;    /*!< Sample time register 0,           0x0C */
    volatile uint32_t SAMPT1;    /*!< Sample time register 1,           0x10 */
    volatile uint32_t resvd1[4];
    volatile uint32_t WDHT;      /*!< Watchdog high threshold register, 0x24 */
    volatile uint32_t WDLT;      /*!< Watchdog low threshold register,  0x28 */
    volatile uint32_t RSQ0;      /*!< Routine sequence register 0,      0x2C */
    volatile uint32_t RSQ1;      /*!< Routine sequence register 1,      0x30 */
    volatile uint32_t RSQ2;      /*!< Routine sequence register 2,      0x34 */
    volatile uint32_t resvd2[5];
    volatile uint32_t RDATA;     /*!< Routine Data register,            0x4C */
    volatile uint32_t resvd3[12];
    volatile uint32_t OVSAMPCTL; /*!< Oversample control register,      0x80 */

    void Enable()  { if((CTL1 & ADC_CTL1_ADCON) == 0) CTL1 |=  ADC_CTL1_ADCON; }
    void Disable() { CTL1 &= ~ADC_CTL1_ADCON; }

    void EnScanMode() { CTL0 |= ADC_CTL0_SM; }
    void EnVrefAndTempChnls() { CTL1 |= ADC_CTL1_TSVREN; }
    void EnExtTrg() { CTL1 |= ADC_CTL1_ETERC; }
    void SelectExtTrg(AdcExtTrgSrc src) { SET_BITS(CTL1, 0b111UL, (uint32_t)src, 17); }
    void EnDMA() { CTL1 |= ADC_CTL1_DMA; }
    void EnOversamping()  { OVSAMPCTL |=  ADC_OVSAMPCTL_OVSEN; }
    void DisOversamping() { OVSAMPCTL &= ~ADC_OVSAMPCTL_OVSEN; }

    void SetupOversampling(AdcOversamplingRatio ratio, AdcOversamplingShift shift) {
        OVSAMPCTL &= ~((0b1111UL << 5) | (0b111UL << 2));
        OVSAMPCTL |= ((uint32_t)shift << 5) | ((uint32_t)ratio << 2);
    }

    void Calibrate() {
        CTL1 |= ADC_CTL1_RSTCLB; // Reset calibration
        while(CTL1 & ADC_CTL1_RSTCLB); // Wait completion
        CTL1 |= ADC_CTL1_CLB; // Perform calibration
        while(CTL1 & ADC_CTL1_CLB); // Wait completion
    }

    void StartConversion() { CTL1 |= ADC_CTL1_SWRCST; } // Set 1 on this bit starts a conversion of a routine sequence if ETSRC is 111

    // Channels setup
    void SetSequenceLength(uint32_t Len) { SET_BITS(RSQ0, 0b1111UL, (Len - 1), 20); }

    void SetChannelSampleTime(uint32_t AChnl, AdcSampleTime ASampleTime) {
        if(AChnl <= 9) SET_BITS(SAMPT1, 0b111UL, (uint32_t)ASampleTime, (AChnl * 3));
        else           SET_BITS(SAMPT0, 0b111UL, (uint32_t)ASampleTime, ((AChnl - 10) * 3));
    }

    void SetSequenceItem(uint32_t SeqIndx, uint32_t AChnl) {
        if     (SeqIndx <= 5)  SET_BITS(RSQ2, 0b11111UL, AChnl, (SeqIndx * 5));
        else if(SeqIndx <= 11) SET_BITS(RSQ1, 0b11111UL, AChnl, ((SeqIndx - 6UL) * 5));
        else if(SeqIndx <= 15) SET_BITS(RSQ0, 0b11111UL, AChnl, ((SeqIndx - 12UL) * 5));
    }

};

#define ADC0    ((ADC_TypeDef*)ADC0_BASE)
#define ADC1    ((ADC_TypeDef*)ADC1_BASE)

#endif

#if 1 // ============================== DAC ====================================
struct DAC_TypeDef {
    volatile uint32_t CTL;        /*!< Control register,                        0x00 */
    volatile uint32_t SWT;        /*!< Software trigger register,               0x04 */
    volatile uint32_t DAC0_R12DH; /*!< DAC0 12-bit right-aligned data register, 0x08 */
    volatile uint32_t DAC0_L12DH; /*!< DAC0 12-bit left-aligned data register,  0x0C */
    volatile uint32_t DAC0_R8DH;  /*!< DAC0 8-bit right-aligned data register,  0x10 */
    volatile uint32_t DAC1_R12DH; /*!< DAC1 12-bit right-aligned data register, 0x14 */
    volatile uint32_t DAC1_L12DH; /*!< DAC1 12-bit left-aligned data register,  0x18 */
    volatile uint32_t DAC1_R8DH;  /*!< DAC1 8-bit right-aligned data register,  0x1C */
    volatile uint32_t DACC_R12DH; /*!< concurrent mode 12-bit right-aligned data register, 0x20 */
    volatile uint32_t DACC_L12DH; /*!< concurrent mode 12-bit left-aligned data register,  0x24 */
    volatile uint32_t DACC_R8DH;  /*!< concurrent mode 8-bit right-aligned data register,  0x28 */
    volatile uint32_t DAC0_DO;    /*!< DAC0 data output register,               0x2C */
    volatile uint32_t DAC1_DO;    /*!< DAC0 data output register,               0x30 */

    enum class Trigger { Tim5TRGO = 0b000, Tim2TRGO = 0b001, Tim6TRGO = 0b010, Tim4TRGO = 0b011,
        Tim1TRGO = 0b100, Tim3TRGO = 0b101, EXTIline9 = 0b110, Software = 0b111 };

    void Enable0()  { CTL |=  (1UL << 0);  }
    void Enable1()  { CTL |=  (1UL << 16); }
    void Disable0() { CTL &= ~(1UL << 0);  }
    void Disable1() { CTL &= ~(1UL << 16); }

    void EnDma0()  { CTL |= 1UL << 12; }
    void EnDma1()  { CTL |= 1UL << 28; }

    void SetTrigger0(Trigger tg) { SET_BITS(CTL, 0b111UL, (uint32_t)tg, 3); }
    void SetTrigger1(Trigger tg) { SET_BITS(CTL, 0b111UL, (uint32_t)tg, 19); }

    void EnTrigger0() { CTL |= 1UL << 2; }
    void EnTrigger1() { CTL |= 1UL << 18; }

    void EnOutBuf0()  { CTL &= ~(1UL << 1); }
    void EnOutBuf1()  { CTL &= ~(1UL << 17); }
    void DisOutBuf0() { CTL |=  (1UL << 1); }
    void DisOutBuf1() { CTL |=  (1UL << 17); }

    void ActivateSwTrigger0() { SWT = (1UL << 0); }
    void ActivateSwTrigger1() { SWT = (1UL << 1); }

    void PutDataR0(uint32_t Data) { DAC0_R12DH = Data; }
    void PutDataL0(uint32_t Data) { DAC0_L12DH = Data; }
};

#define DAC     ((DAC_TypeDef*)DAC_BASE)
#endif

#endif /* LIB_GD32E11X_KL_H_ */
