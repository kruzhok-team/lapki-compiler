
#include "AustereSerialMaster.h"

AustereSerialMaster::AustereSerialMaster(uint8_t misoPin,
                                         uint8_t mosiPin,
                                         uint8_t clkPin,
                                         uint8_t csPin,
                                         uint8_t irqPin)
    : _misoPin(misoPin),
      _mosiPin(mosiPin),
      _clkPin(clkPin),
      _csPin(csPin),
      _irqPin(irqPin),
      _isSetup(false),
      _rxHead(0),
      _rxTail(0),
      _rxCount(0),
      _overrunCount(0) {}

bool AustereSerialMaster::setup() {
  pinMode(_misoPin, INPUT_PULLUP);
  pinMode(_irqPin, INPUT_PULLUP);

  pinMode(_mosiPin, OUTPUT);
  pinMode(_clkPin, OUTPUT);
  pinMode(_csPin, OUTPUT);

  // Idle states per spec
  digitalWrite(_mosiPin, HIGH);
  digitalWrite(_clkPin, HIGH);
  digitalWrite(_csPin, HIGH);

  _rxHead = 0;
  _rxTail = 0;
  _rxCount = 0;
  _overrunCount = 0;
  _isSetup = true;

  return true;
}

bool AustereSerialMaster::sendByte(uint8_t byte) {
  if (!_isSetup) {
    return false;
  }

  uint8_t rxPayload = 0;
  bool rxValid = false;
  if (!transferFrame(true, byte, &rxPayload, &rxValid)) {
    return false;
  }

  if (rxValid) {
    enqueueRx(rxPayload);
  }

  return true;
}

bool AustereSerialMaster::recvByte(uint8_t* out) {
  if (!_isSetup || out == nullptr) {
    // Serial.println("No setup or nullptr");
    return false;
  }

  // Non-blocking: return immediately if already buffered.
  if (dequeueRx(out)) {
    return true;
  }

  // If slave requests service, clock one filler frame.
  if (!irqAsserted()) {
    // Serial.println("irq not asserted");
    return false;
  }

  uint8_t rxPayload = 0;
  bool rxValid = false;
  if (!transferFrame(false, 0x00, &rxPayload, &rxValid)) {
      // Serial.println("smth transfer frame");
      return false;
  }

  if (rxValid) {
    enqueueRx(rxPayload);
  }

  return dequeueRx(out);
}

void AustereSerialMaster::poll() {
  if (!_isSetup || !irqAsserted()) {
    return;
  }

  uint8_t rxPayload = 0;
  bool rxValid = false;
  if (transferFrame(false, 0x00, &rxPayload, &rxValid) && rxValid) {
    enqueueRx(rxPayload);
  }
}

uint8_t AustereSerialMaster::available() const { return _rxCount; }

uint16_t AustereSerialMaster::overrunCount() const { return _overrunCount; }

bool AustereSerialMaster::irqAsserted() const {
  return digitalRead(_irqPin) == LOW;
}

void AustereSerialMaster::clearRx() {
  _rxHead = 0;
  _rxTail = 0;
  _rxCount = 0;
}

bool AustereSerialMaster::transferFrame(bool txValid,
                                        uint8_t txPayload,
                                        uint8_t* rxPayload,
                                        bool* rxValid) {
  if (!_isSetup || rxPayload == nullptr || rxValid == nullptr) {
    return false;
  }

  uint16_t txWord = (static_cast<uint16_t>(txValid ? 0 : 1) << 8) |
                    static_cast<uint16_t>(txPayload);
  uint16_t rxWord = 0;

  // Frame start
  digitalWrite(_csPin, LOW);
  delayMicroseconds(kCsSetupUs);

  for (int8_t bit = 8; bit >= 0; --bit) {
    const uint8_t outBit = (txWord >> bit) & 0x01;

    // CPOL=1, CPHA=1-like behavior:
    // - update on falling edge
    // - sample on rising edge
    digitalWrite(_clkPin, LOW);
    digitalWrite(_mosiPin, outBit ? HIGH : LOW);

    delayMicroseconds(kHalfBitUs);

    digitalWrite(_clkPin, HIGH);
    const uint8_t inBit = digitalRead(_misoPin) ? 1 : 0;
    rxWord = static_cast<uint16_t>((rxWord << 1) | inBit);

    delayMicroseconds(kHalfBitUs);
  }

  // Return outputs to idle levels.
  digitalWrite(_mosiPin, HIGH);
  digitalWrite(_csPin, HIGH);

  delayMicroseconds(kCsHoldUs);

  *rxValid = ((rxWord >> 8) & 0x01) == 0;
  *rxPayload = static_cast<uint8_t>(rxWord & 0xFF);
  return true;
}

void AustereSerialMaster::enqueueRx(uint8_t byte) {
  if (_rxCount >= kRxBufferSize) {
    // discard newest on overrun
    ++_overrunCount;
    return;
  }

  _rxBuffer[_rxHead] = byte;
  _rxHead = static_cast<uint8_t>((_rxHead + 1) % kRxBufferSize);
  ++_rxCount;
}

bool AustereSerialMaster::dequeueRx(uint8_t* out) {
  if (out == nullptr) {
    // Serial.println("nulltprt");
    return false;
  }
  if (_rxCount == 0) {
      // Serial.println("rx count == 0");
      return false;
  }

  *out = _rxBuffer[_rxTail];
  _rxTail = static_cast<uint8_t>((_rxTail + 1) % kRxBufferSize);
  --_rxCount;
  return true;
}
