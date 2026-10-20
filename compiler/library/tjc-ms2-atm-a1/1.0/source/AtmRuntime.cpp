#include "AtmRuntime.hpp"

namespace { volatile uint32_t ticks; }

extern "C" void SystemInit() {
    // M2 is a complete image starting at flash base, unlike the MS1 usercode.
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {}
    RCC->CR &= ~RCC_CR_HSIDIV_Msk;
    RCC->CFGR = 0;
    while (RCC->CFGR & RCC_CFGR_SWS_Msk) {}
    SCB->VTOR = FLASH_BASE;
    SysTick_Config(16000);
}
extern "C" uint32_t millis() { return ticks; }
extern "C" void SysTick_Handler() { ++ticks; }
