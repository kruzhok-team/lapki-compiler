#pragma once
#include "Board.hpp"
#include "AtmI2c.hpp"
#include "SensorDrivers.hpp"

class SHT30 : public atm::HumiditySensor<atm::I2cBus> {
public:
    explicit SHT30(uint8_t position = 5)
        : atm::HumiditySensor<atm::I2cBus>(atm::bus(position == 4 ? 1 : 2)), position(position) {}
    void poll() {
        if (position != 5 && !(position == 4 && atm::hasS4)) {
            if (ready()) failure();
            return;
        }
        atm::HumiditySensor<atm::I2cBus>::poll();
    }
private:
    uint8_t position;
};
class HP203B : public atm::Barometer<atm::I2cBus> {
public:
    HP203B() : atm::Barometer<atm::I2cBus>(atm::bus(1)) {}
};
class SCD41 : public atm::CarbonDioxide<atm::I2cBus> {
public:
    SCD41() : atm::CarbonDioxide<atm::I2cBus>(atm::bus(atm::scd41Bus)) {}
};
class XGZP6859D : public atm::GaugePressure<atm::I2cBus> {
public:
    XGZP6859D() : atm::GaugePressure<atm::I2cBus>(atm::bus(2)) {}
    void poll() {
        if (!atm::hasS6) { if (ready()) failure(); return; }
        atm::GaugePressure<atm::I2cBus>::poll();
    }
};
