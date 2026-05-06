#pragma once

//Скорость UART
#define BAUDRATE 115200
#define ANSWER_SIZE 80

#define ANSWER_NONE 0
#define ANSWER_OK 1
#define ANSWER_ERR 2
#include "stdint.h"
#include "lib/helpers.c"
#include "usb/user.c"

extern "C" {
void
atON
( void )
{
  setPin_PP(GPIOC,1,OFF);
}

void
atOFF
( void )
{
  while ((USART1 -> ISR & USART_ISR_TXE_TXFNF)==0);
  setPin_PP(GPIOC,1,ON);
}

void
setSleep
( bool x )
{
  uint8_t pin;
  if (x) pin = OFF; else pin = ON;
  setPin_PP( GPIOC,3,pin);
}

void initUART ( void ) {
  /* m1.Reg [B.1]
   * ext
       Включаем модуль UART
   * int
       Включаем тактирование USART1 от шины APB
   * tek
       Поднятие бита маской
   */
  RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

  /* m1.Reg [B.2]
   * ext
       Включаем тактирование ног UART
   * int
       Включаем тактирование модуля GPIOA
   * tek
       Поднятие бита маской
   */
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

  /* m1.RegBlock [A]
   * descr
       Настраиваем ноги контроллера для работы с UART
       Мы используем UART2 и ноги PA2, PA3
       Для PA2 значение AF=1
       Для PA3 значение AF=1
    1  PA3: Rx
    2  PA2: Tx
   * links
     - t13 p35 DS12991
   */
  initPin_AF_PP ( GPIOC, 5, 7 ); //[A.1][B.2]
  initPin_AF_PP ( GPIOB, 6, 7 ); //[A.2][B.2]
  initPin_PP ( GPIOC, 1 ); //AT
  initPin_PP ( GPIOC, 3 ); //SLP
  setPin_PP( GPIOC,3,ON);

  /* m1.RegSet #br
   * descr
       Установка скорости UART
   * items
     - Скорость шины APB (задана в system.c:PCLK)
     1 Значение делителя в регистре USARTx_BRR
   * restr
     - Baudrate = PCLK / BRR
     - Иначе BRR = PCLK / Baudrate
   * refs
     - s26.5.7 p794 RM0454
   */
  /* m1.Reg #br.1 [B.3]
   * ext
       Настраиваем скорость UART
       Скорость задана в константе BAUDRATE
   * int
       Устанавливаем делитель частоты USART1
       Он зависит от частоты шины APB
       Её скорость мы задали в system.c константой PCLK
   * tek
       Пишем значение всего регистра
   */
  USART1 -> BRR |= (uint32_t)( PCLK/BAUDRATE );
  USART1 -> CR2 |= USART_CR2_ABREN;
  //USART1 -> CR1 |= USART_CR1_PCE;
  //USART1 -> CR1 |= USART_CR1_PS;

  //USART1 -> CR2 |= USART_CR2_SWAP; //Меняем местами ноги

  /* m1.Reg [B.4]
   * ext
       Включаем прерывание по Rx
   * int
       Включаем прерывание RXNE (Rx buffer not empty)
       Он склеено с прерыванием RXFNE для режима FIFO
       Но этот режим мы не используем
       Более того, он в USART1 не реализован
   * tek
       Поднятие бита маской
   */
  USART1 -> CR1 |= USART_CR1_RXNEIE_RXFNEIE;

  /* m1.Reg [B.5]
   * ext
       Включаем работу модуля
   * int
       Поднимаем бит CR1:UE
   * tek
       Поднятие бита маской
   */
  USART1 -> CR1 |= USART_CR1_UE;

  /* m1.Reg [B.6]
   * ext
       Включаем приёмник
   * int
       Поднимаем бит CR1:RE
   * tek
       Поднятие бита маской
   */
  USART1 -> CR1 |= USART_CR1_RE;

  /* m1.Reg [B.7]
   * ext
       Включаем передатчик
   * int
       Поднимаем бит CR1:TE
   * tek
       Поднятие бита маской
   */
  USART1 -> CR1 |= USART_CR1_TE;

  /* [B.8]
   * Разрешаем прерывание UART2: UART2_IRQn
   */
  NVIC_EnableIRQ(USART1_IRQn);
}

/* Передача одного символа
 * Семантика ReDe вовне этой функции
 * p785 RM0454
 */
void putChar (uint8_t x) {
  /* Если значение регистра TDR ещё не ушло в недра модуля, то надо ждать
   */
  while ((USART1 -> ISR & USART_ISR_TXE_TXFNF)==0);

  /* Отправляем байт
   */
  USART1 -> TDR = x;
}

/* Это отладочная функция, чтобы делать putString("Hell, o world!")
 */
void putString (char *x) {
    int i=0;
    while (x[i] != 0)
        putChar(x[i++]);
}

void
at
( char * cmd
) {
  atON();
  putString("AT+");
  putString(cmd);
  putString("\r\n");
  atOFF();
}


/* Обработчик прерывания USART1
 * p789 RM0454
 */


bool gotAnswer = false;
uint8_t answer[ANSWER_SIZE];
uint32_t answerCnt = 0;
uint32_t answerCode = ANSWER_NONE;

// uint32_t checkAnswer ( void );
void resetEyes (void);

void USART1_IRQHandler (void) {
  /* Читаем пришедшее значение
   * Других источников у этого прерывания нет
   * В хорошей ситуации, должен прийти лишь один байт
  */
  while (USART1->ISR & USART_ISR_RXNE) {
    uint8_t got = USART1 -> RDR;
    answer[0]=got;
  }
  gotAnswer = true;

  USART1 -> RQR |= USART_RQR_RXFRQ; //Не нужно, этот флаг сбрасывается сразу при чтении из RDR
}

bool comp ( uint8_t * compare ) {
    uint32_t i=0;
    while ( compare[i] == answer[i] ) {
        if ( compare[i] == 0 && answer[i] == 0 ) return true;
        if ( compare[i] == 0 || answer[i] == 0 ) return false;
        if ( i == ANSWER_SIZE ) return false;
        i++;
    }
    return false;
};

#define AT_CMD_PAUSE 200
uint32_t
initBT
( void )
{
  uint32_t code = ANSWER_NONE;
  setSleep(OFF);
  delay(30);
  initUART();
  //setSleep(true); delay(100);
  //setSleep(false); delay(100);
  atON();
  delay(AT_CMD_PAUSE);
  putString("AT+RELOAD\r\n");
  delay(AT_CMD_PAUSE);
  putString("AT+NAME=KM-");
  putString(asciiSerial());
  putString("\r\n");
  delay(AT_CMD_PAUSE);
  //delay(5); if (answerCode != ANSWER_OK) return answerCode;
  putString("AT+PNAME=blg-mb-1-a12\r\n");
  delay(AT_CMD_PAUSE);
  //delay(5); if (answerCode == ANSWER_OK) return answerCode;
  putString("AT+PASS=665816\r\n");
  delay(AT_CMD_PAUSE);
  //delay(2); if (answerCode == ANSWER_OK) return answerCode;
  putString("AT+PASEN=OFF\r\n");
  delay(AT_CMD_PAUSE);
  //delay(2); if (answerCode == ANSWER_OK) return answerCode;
  putString("AT+RESET\r\n");
  delay(AT_CMD_PAUSE);
  atOFF();
  return ANSWER_OK;
}
}
