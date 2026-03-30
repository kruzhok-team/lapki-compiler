#ifndef AUSTERE_SERIAL_MASTER_H
#define AUSTERE_SERIAL_MASTER_H

#include <Arduino.h>
#include <stdint.h>

class AustereSerialMaster {
 public:
  static const uint8_t kRxBufferSize = 32;

  AustereSerialMaster(uint8_t misoPin,
                      uint8_t mosiPin,
                      uint8_t clkPin,
                      uint8_t csPin,
                      uint8_t irqPin);

  // Required API
  bool setup();
  bool sendByte(uint8_t byte);
  bool recvByte(uint8_t* out);

  // Optional helpers
  void poll();
  uint8_t available() const;
  uint16_t overrunCount() const;
  bool irqAsserted() const;
  void clearRx();

 private:
  static const uint16_t kBitPeriodUs = 500;   // 2 kHz
  static const uint16_t kHalfBitUs = 250;
  static const uint16_t kCsSetupUs = 100;
  static const uint16_t kCsHoldUs = 100;

  bool transferFrame(bool txValid, uint8_t txPayload, uint8_t* rxPayload, bool* rxValid);
  void enqueueRx(uint8_t byte);
  bool dequeueRx(uint8_t* out);

  uint8_t _misoPin;
  uint8_t _mosiPin;
  uint8_t _clkPin;
  uint8_t _csPin;
  uint8_t _irqPin;

  bool _isSetup;

  uint8_t _rxBuffer[kRxBufferSize];
  uint8_t _rxHead;
  uint8_t _rxTail;
  uint8_t _rxCount;
  uint16_t _overrunCount;
};

#endif  // AUSTERE_SERIAL_MASTER_H
