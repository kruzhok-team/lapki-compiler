#pragma once

#define LOAD_ON 10
#define LOAD_ALL (1e3*15*1-LOAD_ON)

volatile uint32_t loadTimer;

__attribute__((always_inline)) 
static inline
uint8_t
load
( uint8_t x ) {
  return setPin_PP ( GPIOC, 0, x );
}

volatile uint32_t loadOn = LOAD_ON;
volatile uint32_t loadAll = LOAD_ALL;

void
initLoad
( uint32_t on //в мс
, uint32_t all //в с
) {

  all = all ? all : 1;
  on  = on  ? on  : 1;

  loadOn = on;
  loadAll = 1e3*all - on;
  initPin_AF_PP(GPIOC,0,2);
  //load(OFF);

  RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;
  
  TIM1 -> BDTR |= TIM_BDTR_MOE;
  TIM1 -> ARR = 500*all;
  TIM1 -> PSC = 71;

  TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // ШИМ режим 1 (OC1M = 110) | TIM_CCMR1_OC2M_0	
	TIM1->CCER  |= TIM_CCER_CC1E;                       // Разрешаем вывод PWM сигнала на PORT PA8 N11
  TIM1->CCMR1 |= TIM_CCMR1_OC1PE;   // Включаем регистр предварительной загрузки компаратора   канала 1
  TIM1 -> CCR1 = on;

  TIM1->CR1 &= ~TIM_CR1_UDIS;   // Разрешить обновление
  TIM1->CR1 &= ~TIM_CR1_DIR;    // считаем вверх !
  TIM1->CR1 |= TIM_CR1_ARPE;    // Буферизация TIM->ARR  ;
  TIM1->CR1 &= ~TIM_CR1_CMS;    // TIM3->CR1&= ~TIM_CR1_CMS_0;
  TIM1->EGR   =  TIM_EGR_UG;    // Сброс PSC и CNT, без этого не работает
  TIM1->SR   &= ~TIM_SR_UIF;    // Сбросили флаг прерывания
  TIM1->CR1 |= TIM_CR1_CEN;
  //NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void
TIM1_DAC_IRQHandler
( void )
{
  //TIM6 -> CR1 &= ~TIM_CR1_CEN;
  TIM1 -> SR &= ~TIM_SR_UIF;
  manageLoad();
  //TIM6 -> CR1 |= TIM_CR1_CEN;
}

volatile uint32_t cycleCounter = 0;

void
manageLoad
( void )
{
  if (loadTimer < loadOn ) {
    load(ON);
    //led_LR(ON);
    //led_RR(ON);
    //led_LG(OFF);
    //led_RG(OFF);
  }
  else {
    load(OFF);
    //led_LR(OFF);
    //led_RR(OFF);
    //led_LG(ON);
    //led_RG(ON);
  }
  
  if ( loadTimer >= loadAll ) {
    loadTimer = 0;
    cycleCounter %= 35;
    cycleCounter++;
    for (uint8_t i=0; i<cycleCounter; i++) {
      //setMxLed(*mxByRows[i],ON);
    }
  }
  else {
    loadTimer++;
  }
}
