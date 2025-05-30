/*
 * board.h
 *
 *  Created on: 28.08.2023
 *      Author: Kreyl
 */

#ifndef BOARD_H__
#define BOARD_H__

// ==== General ====
#define BOARD_NAME          "IR_PCBv2"
#define APP_NAME            "IR_PCB"

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ             12000000UL

#if 1 // ==== MCU info ====
// Configuration of the Cortex-M4 processor and core peripherals
#define __CM4_REV                   0x0001   // Core revision r0p1
#define __MPU_PRESENT               0        // GD32E11x do not provide MPU
#define CORTEX_SIMPLIFIED_PRIORITY  FALSE
#define CORTEX_PRIORITY_BITS        4        // GD32E11x uses 4 bits for the priority levels
#define __FPU_PRESENT               1        // FPU present

#define CORTEX_USE_FPU              __FPU_PRESENT
#define CORTEX_MODEL                4
#endif

#if 1 // ==== OS timer settings ====
#define SYS_TIM                     TIM10
#define SYS_TIM_IRQn                TIMER0_TRG_CMT_TIMER10_IRQn
#define SYS_TIM_IRQ_HANDLER         TIMER0_TRG_CMT_TIMER10_IRQHandler
#define SYS_TIM_IRQ_PRIORITY        2
#endif

#if 1 // ========================== GPIO =======================================
// EXTI
#define INDIVIDUAL_EXTI_IRQ_REQUIRED    FALSE

// User GPIOs. Hardware-dependent.
#define Gpio1           PB2
#define Gpio2           PB10
#define Gpio3           PB11
#define Gpio4           PA8

// Usage by app
//#define Output_PulseOnHit
#define Output_HitsPresent  Gpio3
#define Input_BurstFire     Gpio2, Gpio::PullDown
#define Input_SingleFire    Gpio1, Gpio::PullDown
#define INPUT_DEADTTIME_ms  36
// PWM Input
#define INPUT_PWM           PA8, Gpio::PullDown, TIM0
#define INPUT_PWM_TMR_IRQ   TIMER0_Channel_IRQn
#define INPUT_PWM_TMR_IRQ_HNDLR TIMER0_Channel_IRQHandler
#define INPUT_PWM_CHECK_PERIOD_ms       27 // It is timeout, too. After this period of pulse absense there will be event of Duty=0
#define PWM_DUTY_SINGLE_FIRE_percent    33L
#define PWM_DUTY_BURST_FIRE_percent     66L
#define PWM_DUTY_DEVIATION_percent      9L // PWM is considered unchanged if the change is less than this value

// UART
#define UART_TX_PIN     PA2
#define UART_RX_PIN     PA3

// Green LED
#define LUMOS_PIN       PA10, Gpio::PushPull

// Front LEDs
#define LED_FRONT1      { PB8, TIM3, 2, invNotInverted, Gpio::PushPull, 255 }
#define LED_FRONT2      { PB9, TIM3, 3, invNotInverted, Gpio::PushPull, 255 }
#define FRONT_LEDS_CNT  2

// Side LEDs
#define LED_PWM1        { PA6, TIM2, 0, invInverted, Gpio::PushPull, 255 }
#define LED_PWM2        { PA7, TIM2, 1, invInverted, Gpio::PushPull, 255 }
#define LED_PWM3        { PB0, TIM2, 2, invInverted, Gpio::PushPull, 255 }
#define LED_PWM4        { PB1, TIM2, 3, invInverted, Gpio::PushPull, 255 }
#define SIDE_LEDS_CNT   4

// Neopixel LEDs
#define NPX_PARAMS      PA0, TIM4, 0

// Beeper
#define BEEPER_TOP      22 // 22 < 255: needed to increase frequency
#define BEEPER_PIN      { PB14, TIM11, 0, invInverted, Gpio::PushPull, BEEPER_TOP }

// IR LED
#define IR_LED          PA4 // DAC
#define IR_BIT_CNT_MAX  16L // Just for buffer reservation

// IR Rcvr
#define IR_RX_DATA_PIN  PA3

// Spi Flash
#define FLASH_NSS       PA15
#define FLASH_SCK       PB3
#define FLASH_MISO      PB4
#define FLASH_MOSI      PB5
#define FLASH_IO2       PB6
#define FLASH_IO3       PB7

// Audio
#define AU_SPI          SPI1
#define AU_SDMODE       PC13
#define AU_LRCK         PB12
#define AU_BCLK         PB13
#define AU_SDIN         PB15

#endif // GPIO

#if 1 // ========================= Timer =======================================
// IR LED
#define TMR_DAC_SMPL        TIM6
// IR Receiver
#define TMR_IR_RX           TIM1
#define TMR_IR_RX_IRQ       TIMER1_IRQn
#define TMR_IR_RX_IRQ_HNDLR TIMER1_IRQHandler
#endif // Timer

#if 1 // ==== USB ====
#define USB_SOF_CB_EN       FALSE // SOF callback not used
#define USB_IRQ_PRIO        14
#define USB_TXBUF_CNT       16   // 16 buffers of size=EP_BULK_SZ=64 each results in 1024 bytes total
// Crystalless mode: IRC48M utilized, it syncs using SOF pulses at PA8. Therefore, PA8 is occupied.
#define USB_CRYSTALLESS_EN  TRUE
// Next options: set to 1U if enabled, 0U if disabled. Required for Setup Request reply.
#define USB_REMOTE_WKUP_EN  0U
#define USB_SELF_POWERED    0U  // if not self powered, then it is bus powered
// Hardware depended, do not touch
#define USB_SET_ADDRESS_ACK_BY_HW   FALSE
#define USB_SEQUENCE_WORKAROUND     TRUE    // XXX CheckMe
#define USB_SET_ADDR_AFTER_ZEROPKT  FALSE
#define USB_NUM_EP_MAX              3U // Excluding Ep0
#define USB_IRQ_HANDLER             USBFS_IRQHandler // 0x14C
#define USB_IRQ_NUMBER              USBFS_IRQn // 67

#endif

#if 1 // =========================== I2C =======================================
#define I2C0_ENABLED    FALSE
#define I2C1_ENABLED    FALSE

#define I2C_BAUDRATE_HZ 400000UL
#define I2C_DUTYCYC     I2C_DUTYCYCLE_2
#endif

#if 1 // ======================== Inner ADC ====================================
#define ADC_REQUIRED    FALSE

#endif

#if 1 // =========================== DMA =======================================
// SPI FLASH
#define SPIFLASH_DMA_RX     DMA0_Channel1
#define SPIFLASH_DMA_TX     DMA0_Channel2
// Audio
#define I2S_DMA_TX          DMA0_Channel4
// Uart
#define UART_DMA_RX         DMA0_Channel5
#define UART_DMA_TX         DMA0_Channel6
// Npx LEDs
#define NPX_DMA             DMA1_Channel1 // Tim4 Update
// DAC
#define DAC_DMA             DMA1_Channel2

#endif // DMA

#if 1 // ========================== USART ======================================
//#define PRINTF_FLOAT_EN TRUE
#define UART_TXBUF_SZ   3000
#define UART_RXBUF_SZ   128
#define CMD_BUF_SZ      128
#define UART_RX_POLL_MS 99

#define CMD_UART        USART1

#define CMD_UART_PARAMS CMD_UART, UART_TX_PIN, UART_RX_PIN, UART_DMA_TX, UART_DMA_RX

#endif

#endif //BOARD_H__
