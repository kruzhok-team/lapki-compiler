#pragma once

#include "user.c"

void Rx_HandleSystem(char ep, short length) {
  USB_SetEP((short*)(&USB->EP0R)+ep*2, USB_EP_RX_NAK , USB_EP_RX_VALID );
  USB_Fetch(ep, (char*)bufSystemQ, &lenSystemQ);
  flushSystemA();
  processQ();
  flushSystemQ();
  USB_Transmit(ep,(char*)bufSystemA, lenSystemA);
  USB_SetEP((short*)(&USB->EP0R)+ep*2, USB_EP_RX_VALID , USB_EP_RX_VALID );
/*
  // do NOT busy wait. We are still in the ISR, it will never clear.
  // Срабатывает, если дравйвер думает, что передача окончена (пустой пакет или его размер меньше 64 байт)
  if (!USB_IsTransmitPending(ep)) {
  }
*/
}

uint8_t userData [64];

void Rx_HandleData(char ep, short length) {
  USB_SetEP((short*)(&USB->EP0R)+ep*2, USB_EP_RX_NAK , USB_EP_RX_VALID );

  USB_Fetch(ep, (char*)bufDataQ, &lenDataQ);
  flushDataA();
  
  popUserQueue(&lenDataA,bufDataA);

  //flushDataQ();

  USB_Transmit(ep,(char*)bufDataA,lenDataA);

  USB_SetEP((short*)(&USB->EP0R)+ep*2, USB_EP_RX_VALID , USB_EP_RX_VALID );
  /*
  USB_Fetch(ep, (char*)buffer, &length);
  //if (length > 0) {
  if (1) {
    USB_Transmit(ep, (char*)userData, 63);
    return;
  }

  // do NOT busy wait. We are still in the ISR, it will never clear.
  // Срабатывает, если дравйвер думает, что передача окончена (пустой пакет или его размер меньше 64 байт)
  if (!USB_IsTransmitPending(ep)) {
  }
  */
}
