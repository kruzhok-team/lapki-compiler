#define SYSCLOCK 144e6
#define HCLK 72e6
#define PCLK 72e6
#define PCLK2 72e6

#define FLASH_BASE_ADDR 0x0800a000U  //устанавливаем адрес приложения

extern "C" {
  void manageLoad( void ); 
  void initClock (void) {
    RCC -> CR |= RCC_CR_HSEON; //Set HSI on (will use it as temporary source)
    RCC -> CRRCR |= RCC_CRRCR_HSI48ON;
    while ((RCC -> CR & RCC_CR_HSERDY) == 0); //Wait until its ready
    RCC -> CFGR = ((RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSE); //Switch to HSI
    RCC -> CRRCR |= RCC_CRRCR_HSI48ON;
    RCC -> CFGR = (( RCC->CFGR & ~RCC_CFGR_HPRE ) | ( 0b1000 < RCC_CFGR_HPRE_Pos )); //NB s7.2.7 p282 RM0440
    RCC -> PLLCFGR = (( RCC->PLLCFGR & ~RCC_PLLCFGR_PLLSRC_Msk ) | RCC_PLLCFGR_PLLSRC_HSE );
    RCC -> PLLCFGR = (( RCC->PLLCFGR & ~RCC_PLLCFGR_PLLN_Msk ) | ( 18 << RCC_PLLCFGR_PLLN_Pos ));
    RCC -> PLLCFGR = (( RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM_Msk ) | ( (1-1) << RCC_PLLCFGR_PLLM_Pos ));
    RCC -> PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC -> PLLCFGR |= RCC_PLLCFGR_PLLPEN;
    RCC -> PLLCFGR |= RCC_PLLCFGR_PLLQEN;
    RCC -> CR |= RCC_CR_PLLON;
    while ( RCC->CR & RCC_CR_PLLRDY);
    RCC -> CFGR = ((RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL); //Switch clock source to PLL
    RCC -> CR &= ~RCC_CR_HSION;
  }
  
  void SystemInit ( void ) {
    SCB->VTOR = FLASH_BASE_ADDR;  //установка VECT_TAB_OFFSET
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2)); // Включение FPU

    __enable_irq();  //включаем все прерывания TODO
    FLASH -> SEC1R |= FLASH_SEC1R_BOOT_LOCK;
    FLASH -> OPTR |= FLASH_OPTR_nBOOT0;
    FLASH -> OPTR &= ~FLASH_OPTR_nSWBOOT0;
    FLASH -> ACR |= ( 0b0011 << FLASH_ACR_LATENCY_Pos ); //0b0011=3ws, p216 RM0440
    //initClock();
    SysTick_Config ((uint32_t)(HCLK/1000));
  }
  
  volatile uint32_t waiter = 0;
  
  __attribute__((optimize("O0"))) 
  void delay ( uint32_t wait ) {
    waiter = wait;
    while (waiter);
  }
  
  volatile uint32_t counter = 0;
  volatile uint32_t blinkTimeout = 0;
  volatile uint32_t roarTimer = 0;
  volatile uint32_t matrixTimer = 0;
  volatile uint32_t starTimer = 0;
  volatile uint32_t beepTimer = 0;
  volatile uint32_t accelTimerStart = 0;
  volatile uint32_t accelTimerUpdate = 0;
  volatile uint32_t rebootAfter = 0;
  
  __attribute__((optimize("O0")))
  uint32_t millis() {
    return counter;
  }
  
  void reset ( void );
  
  void SysTick_Handler ( void ) {
    switch ( rebootAfter ) {
      case 0: break;
      case 1: reset(); break;
      default: rebootAfter--; break;
    }
    !waiter ?: waiter--;
    !blinkTimeout ?: blinkTimeout--;
    !roarTimer ?: roarTimer--;
    !matrixTimer ?: matrixTimer--;
    !starTimer ?: starTimer--;
    !beepTimer ?: beepTimer--;
    !accelTimerStart ?: accelTimerStart--;
    !accelTimerUpdate ?: accelTimerUpdate--;
    ++counter;
    //manageLoad(); 
  }
}
