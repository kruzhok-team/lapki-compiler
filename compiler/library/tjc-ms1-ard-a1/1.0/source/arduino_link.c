
#define LINK_RX_BUFFER_SIZE 32u
#define LINK_TX_BUFFER_SIZE 32u
#define LINK_RX_MASK (LINK_RX_BUFFER_SIZE - 1u)
#define LINK_TX_MASK (LINK_TX_BUFFER_SIZE - 1u)

#if ((LINK_RX_BUFFER_SIZE & LINK_RX_MASK) != 0)
#error "LINK_RX_BUFFER_SIZE must be power-of-two"
#endif

#if ((LINK_TX_BUFFER_SIZE & LINK_TX_MASK) != 0)
#error "LINK_TX_BUFFER_SIZE must be power-of-two"
#endif

extern "C" {

typedef struct {
  // RX queue: producer=ISR, consumer=main
  uint8_t rx_buf[LINK_RX_BUFFER_SIZE];
  volatile uint16_t rx_head;
  volatile uint16_t rx_tail;

  // TX queue: producer=main, consumer=ISR
  uint8_t tx_buf[LINK_TX_BUFFER_SIZE];
  volatile uint16_t tx_head;
  volatile uint16_t tx_tail;

  // Diagnostics
  volatile uint16_t rx_overrun;
  volatile uint16_t tx_drop;

  // Per-frame state (9-bit MSB-first)
  volatile uint16_t rx_word;
  volatile uint16_t tx_word;
  volatile uint8_t bit_index;  // 0..9 (9 means frame complete)
  volatile uint8_t in_frame;
} link_state_t;

static link_state_t sm = {0};

//
// Pin map:
// MISO 1 PA9  (slave -> master)
// MOSI 2 PC6  (master -> slave)
// CLK  3 PA10 (master clock)
// CS   4 PA11  (frame select, active low)
// IRQ  5 PA12  (slave request, active low)
//

static inline void pin_miso_write(int value) {
  setPin_OD(GPIOA, 9, value ? OFF : ON);
}

static inline int pin_mosi_read(void) {
  return readPin(GPIOC, 6);
}

static inline int pin_clk_read(void) {
  return readPin(GPIOA, 10);
}

static inline int pin_cs_read(void) {
  return readPin(GPIOA, 11);
}

static inline void pin_irq_write(int value) {
  setPin_OD(GPIOA, 12, value ? OFF : ON);
}

static int tx_fifo_empty(void) {
  return sm.tx_head == sm.tx_tail;
}

static inline void refresh_irq_level(void) {
  // Active-low IRQ asserted when at least one TX byte is queued.
  pin_irq_write(tx_fifo_empty() ? 1 : 0);
}

static inline void rx_fifo_push(uint8_t b) {
  const uint16_t head = sm.rx_head;
  const uint16_t tail = sm.rx_tail;

  if ((uint16_t)(head - tail) >= LINK_RX_BUFFER_SIZE) {
    // Spec: discard newest on overrun.
    sm.rx_overrun++;
    return;
  }

  sm.rx_buf[head & LINK_RX_MASK] = b;
  sm.rx_head = (uint16_t)(head + 1u);
}

static inline int rx_fifo_pop(uint8_t* out_b) {
  if (out_b == 0) {
    return 0;
  }

  const uint16_t tail = sm.rx_tail;
  if (tail == sm.rx_head) {
    return 0;
  }

  *out_b = sm.rx_buf[tail & LINK_RX_MASK];
  sm.rx_tail = (uint16_t)(tail + 1u);
  return 1;
}

static inline int tx_fifo_push(uint8_t b) {
  const uint16_t head = sm.tx_head;
  const uint16_t tail = sm.tx_tail;

  if ((uint16_t)(head - tail) >= LINK_TX_BUFFER_SIZE) {
    sm.tx_drop++;
    return 0;
  }

  sm.tx_buf[head & LINK_TX_MASK] = b;
  sm.tx_head = (uint16_t)(head + 1u);
  return 1;
}

static inline int tx_fifo_pop(uint8_t* out_b) {
  if (out_b == 0) {
    return 0;
  }

  const uint16_t tail = sm.tx_tail;
  if (tail == sm.tx_head) {
    return 0;
  }

  *out_b = sm.tx_buf[tail & LINK_TX_MASK];
  sm.tx_tail = (uint16_t)(tail + 1u);
  return 1;
}

static inline uint16_t build_next_tx_word(void) {
  uint8_t payload = 0;
  if (tx_fifo_pop(&payload)) {
    return (uint16_t)(0x000u | (uint16_t)payload);  // DV=true + payload
  }
  return 0x100u;  // DV=false + 0x00 filler
}

static inline void set_miso_for_bit_index(uint8_t bit_index) {
  const uint8_t bit = (uint8_t)((sm.tx_word >> (8u - bit_index)) & 0x1u);
  pin_miso_write(bit ? 1 : 0);
}

static inline void frame_start(void) {
  sm.bit_index = 0;
  sm.rx_word = 0;
  sm.tx_word = build_next_tx_word();

  // Preload bit8 (DV), then on subsequent falling edges set bit7..bit0.
  set_miso_for_bit_index(0u);
  refresh_irq_level();
}

static inline void frame_complete_rx(void) {
  const uint8_t dv = (uint8_t)((sm.rx_word >> 8) & 0x1u);
  const uint8_t payload = (uint8_t)(sm.rx_word & 0xFFu);
  if (dv == 0) {
    rx_fifo_push(payload);
  }
}

static inline void on_cs_asserted(void) {
  sm.in_frame = 1u;
  frame_start();
}

static inline void on_cs_deasserted(void) {
  if (!sm.in_frame) {
    return;
  }

  // If bit_index < 9, frame was partial and is discarded.
  sm.in_frame = 0u;
  sm.bit_index = 0u;
  sm.rx_word = 0u;
  sm.tx_word = 0u;

  // Idle high.
  pin_miso_write(1);
  refresh_irq_level();
}

static inline void on_clk_edge(void) {
  if (!sm.in_frame || pin_cs_read()) {
    return;
  }

  // CPOL=1, CPHA=1-like:
  // - update output on falling edge
  // - sample input on rising edge
  if (!pin_clk_read()) {
    // Falling edge
    if (sm.bit_index == 9u) {
      // Burst mode: previous 9-bit frame complete; start next frame while CS stays low.
      frame_start();
      return;
    }

    // bit_index==0 already preloaded at frame_start().
    if (sm.bit_index > 0u && sm.bit_index < 9u) {
      set_miso_for_bit_index((uint8_t)sm.bit_index);
    }
    return;
  }

  // Rising edge: sample MOSI
  if (sm.bit_index < 9u) {
    sm.rx_word = (uint16_t)((sm.rx_word << 1) | (uint16_t)(pin_mosi_read() ? 1u : 0u));
    sm.bit_index++;

    if (sm.bit_index == 9u) {
      frame_complete_rx();
    }
  }
}

//
// Arduino interface pin + state machine init
//
void arduino_link_init(void) {
  RCC -> APBENR2 |= RCC_APBENR2_SYSCFGEN; // EXTI enable
  RCC -> IOPENR |= RCC_IOPENR_GPIOAEN; // Bank enable
  RCC -> IOPENR |= RCC_IOPENR_GPIOCEN; // Bank enable

  initPin_OD( GPIOA, 9 );  // MISO
  initPin_InputF( GPIOC, 6 );    // MOSI
  initPin_InputF( GPIOA, 10 );  // CLK
  initPin_InputF( GPIOA, 11 );  // CS
  initPin_OD( GPIOA, 12 ); // IRQ

  // PA10, PA11 interrupts
  EXTI->EXTICR[2] &= ~(EXTI_EXTICR3_EXTI10_Msk | EXTI_EXTICR3_EXTI11_Msk);
  EXTI->EXTICR[2] &= ~(EXTI_EXTICR3_EXTI10_Msk | EXTI_EXTICR3_EXTI11_Msk);

  // Falling edge trigger
  EXTI->FTSR1 |= EXTI_FTSR1_FT10;
  EXTI->FTSR1 |= EXTI_FTSR1_FT11;

  // Rising edge trigger
  EXTI->RTSR1 |= EXTI_RTSR1_RT10;
  EXTI->RTSR1 |= EXTI_RTSR1_RT11;

  // Unmask Interrupt
  EXTI->IMR1 |= EXTI_IMR1_IM10;
  EXTI->IMR1 |= EXTI_IMR1_IM11;

  NVIC_SetPriority(EXTI4_15_IRQn, 15);
  NVIC_EnableIRQ(EXTI4_15_IRQn);

  sm.rx_head = 0u;
  sm.rx_tail = 0u;
  sm.tx_head = 0u;
  sm.tx_tail = 0u;
  sm.rx_overrun = 0u;
  sm.tx_drop = 0u;
  sm.rx_word = 0u;
  sm.tx_word = 0u;
  sm.bit_index = 0u;
  sm.in_frame = 0u;

  // Idle high
  pin_miso_write(1);
  pin_irq_write(1);
}

int arduino_link_send_byte(uint8_t byte) {
  const int ok = tx_fifo_push(byte);
  refresh_irq_level();
  return ok;
}

int arduino_link_recv_byte(uint8_t* out_byte) {
  return rx_fifo_pop(out_byte);
}

uint16_t arduino_link_rx_overrun_count(void) {
  return sm.rx_overrun;
}

uint16_t arduino_link_tx_drop_count(void) {
  return sm.tx_drop;
}

void EXTI4_15_IRQHandler() {
  if (EXTI->RPR1 & EXTI_RPR1_RPIF10) { // CLK Rise
    EXTI->RPR1 = EXTI_RPR1_RPIF10;
    on_clk_edge();
  }

  if (EXTI->FPR1 & EXTI_FPR1_FPIF10) { // CLK Fall
    EXTI->FPR1 = EXTI_FPR1_FPIF10;
    on_clk_edge();
  }

  if (EXTI->RPR1 & EXTI_RPR1_RPIF11) { // CS Rise
    EXTI->RPR1 = EXTI_RPR1_RPIF11;
    on_cs_deasserted();
  }

  if (EXTI->FPR1 & EXTI_FPR1_FPIF11) { // CS Fall
    EXTI->FPR1 = EXTI_FPR1_FPIF11;
    on_cs_asserted();
  }
}
}
