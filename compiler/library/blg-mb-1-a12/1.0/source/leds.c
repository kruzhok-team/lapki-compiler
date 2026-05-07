#ifdef _CODE_PINS
#else
#define _CODE_PINS

//#include "const.h"
#include "lib/pins.c"
#define MX_SIZE 35

__attribute__((always_inline)) 
static inline
uint8_t
ledSys
( uint8_t x ) {
  return setPin_OD ( GPIOB, 1, x );
}

void
initLedSys
( void )
{
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  initPin_OD(GPIOB,1);
}

typedef struct MxLed {
  uint16_t value;
  GPIO_TypeDef (*port);
  uint8_t number;
} MxLed;

void
initMxLed
( MxLed mx
) {
  initPin_OD ( mx.port, mx.number );
}

void
setMxLed
( MxLed mx
, uint8_t set
) {
  setPin_OD ( mx.port, mx.number, set );
}

MxLed mx_a1 = { 0, GPIOE,1 };
MxLed mx_a2 = { 0, GPIOE,0 };
MxLed mx_a3 = { 0, GPIOB,9 };
MxLed mx_a4 = { 0, GPIOB,5 };
MxLed mx_a5 = { 0, GPIOE,6 };
MxLed mx_a6 = { 0, GPIOC,13};
MxLed mx_a7 = { 0, GPIOF,10};

MxLed mx_b1 = { 0, GPIOD,4 };
MxLed mx_b2 = { 0, GPIOE,2 };
MxLed mx_b3 = { 0, GPIOE,3 };
MxLed mx_b4 = { 0, GPIOE,4 };
MxLed mx_b5 = { 0, GPIOE,5 };
MxLed mx_b6 = { 0, GPIOD,6 };
MxLed mx_b7 = { 0, GPIOF,9 };

MxLed mx_c1 = { 0, GPIOD,1 };
MxLed mx_c2 = { 0, GPIOD,3 };
MxLed mx_c3 = { 0, GPIOC,12};
MxLed mx_c4 = { 0, GPIOD,7 };
MxLed mx_c5 = { 0, GPIOB,3 };
MxLed mx_c6 = { 0, GPIOD,12};
MxLed mx_c7 = { 0, GPIOD,11};

MxLed mx_d1 = { 0, GPIOD,0 };
MxLed mx_d2 = { 0, GPIOD,2 };
MxLed mx_d3 = { 0, GPIOC,11};
MxLed mx_d4 = { 0, GPIOC,10};
MxLed mx_d5 = { 0, GPIOD,5 };
MxLed mx_d6 = { 0, GPIOD,13};
MxLed mx_d7 = { 0, GPIOD,15};

MxLed mx_e1 = { 0, GPIOA,10};
MxLed mx_e2 = { 0, GPIOA,9 };
MxLed mx_e3 = { 0, GPIOA,8 };
MxLed mx_e4 = { 0, GPIOC,9 };
MxLed mx_e5 = { 0, GPIOC,8 };
MxLed mx_e6 = { 0, GPIOC,7 };
MxLed mx_e7 = { 0, GPIOC,6 };

static MxLed * mxMirror[7][5] =
  {{ &mx_a1, &mx_b1, &mx_c1, &mx_d1, &mx_e1 }
  ,{ &mx_a2, &mx_b2, &mx_c2, &mx_d2, &mx_e2 }
  ,{ &mx_a3, &mx_b3, &mx_c3, &mx_d3, &mx_e3 }
  ,{ &mx_a4, &mx_b4, &mx_c4, &mx_d4, &mx_e4 }
  ,{ &mx_a5, &mx_b5, &mx_c5, &mx_d5, &mx_e5 }
  ,{ &mx_a6, &mx_b6, &mx_c6, &mx_d6, &mx_e6 }
  ,{ &mx_a7, &mx_b7, &mx_c7, &mx_d7, &mx_e7 }
  };

static MxLed * mxByRows[35] =
  { &mx_a1, &mx_b1, &mx_c1, &mx_d1, &mx_e1
  , &mx_a2, &mx_b2, &mx_c2, &mx_d2, &mx_e2
  , &mx_a3, &mx_b3, &mx_c3, &mx_d3, &mx_e3
  , &mx_a4, &mx_b4, &mx_c4, &mx_d4, &mx_e4
  , &mx_a5, &mx_b5, &mx_c5, &mx_d5, &mx_e5
  , &mx_a6, &mx_b6, &mx_c6, &mx_d6, &mx_e6
  , &mx_a7, &mx_b7, &mx_c7, &mx_d7, &mx_e7
  };

static MxLed * mxByCols[35] =
  { &mx_a1 , &mx_a2 , &mx_a3 , &mx_a4 , &mx_a5 , &mx_a6 , &mx_a7
  , &mx_b1 , &mx_b2 , &mx_b3 , &mx_b4 , &mx_b5 , &mx_b6 , &mx_b7
  , &mx_c1 , &mx_c2 , &mx_c3 , &mx_c4 , &mx_c5 , &mx_c6 , &mx_c7
  , &mx_d1 , &mx_d2 , &mx_d3 , &mx_d4 , &mx_d5 , &mx_d6 , &mx_d7
  , &mx_e1 , &mx_e2 , &mx_e3 , &mx_e4 , &mx_e5 , &mx_e6 , &mx_e7
  };

void
initLeds
( void ) {
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIODEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOFEN;

  initLedSys();
  ledSys(ON);
  
  for (uint8_t i=0; i<MX_SIZE; i++) {
    initMxLed(*mxByRows[i]);
  }
}
#endif
