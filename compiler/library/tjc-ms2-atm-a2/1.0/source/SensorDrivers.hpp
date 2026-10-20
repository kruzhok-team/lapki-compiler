#pragma once
#include "SensorMath.hpp"

namespace atm {
// Transport supplies write/read and a wraparound uint32_t millisecond clock.
// Kept independent of the MCU so protocol/recovery can be exercised on a host.
template<class Transport> class Sensor {
protected:
    Transport& io;
    uint8_t address, state = 0;
    uint32_t at = 0, limit = 0;
    bool received = false, readError = false;
    bool ready() const { return int32_t(Transport::now() - at) >= 0; }
    bool expired() const { return int32_t(Transport::now() - limit) >= 0; }
    bool command(uint8_t cmd) { return io.write(address, &cmd, 1); }
    bool command16(uint16_t cmd) {
        const uint8_t bytes[] = {uint8_t(cmd >> 8), uint8_t(cmd)};
        return io.write(address, bytes, 2);
    }
    bool reply(uint8_t cmd, uint8_t* bytes, uint8_t count) {
        return command(cmd) && io.read(address, bytes, count);
    }
    void success() {
        error = false;
        received = true;
        state = 0;
        at = Transport::now() + 1000;
    }
    void failure() {
        error = true;
        readError = true;
        state = 0;
        at = Transport::now() + 1000;
    }
public:
    // Last read attempt failed. No validity/freshness policy is imposed.
    bool error = false;
    Sensor(Transport& transport, uint8_t addr) : io(transport), address(addr) {}
    void begin() {
        state = 0;
        at = Transport::now() + 30;
        received = readError = error = false;
    }
    bool valueReceived() { const bool result = received; received = false; return result; }
    bool readingError() { const bool result = readError; readError = false; return result; }
};

// S4 NSHT30-CLAR and S5 SHT30 share the single-shot command and CRC format.
template<class Transport> class HumiditySensor : public Sensor<Transport> {
    using Base = Sensor<Transport>;
    bool initialized = false;
    const uint8_t configuredAddress;
    void bad() { initialized = false; Base::failure(); }
public:
    int32_t temperature = 0, humidity = 0;
    // addr=0 discovers 0x44/0x45 on this sensor's board-assigned bus.
    HumiditySensor(Transport& transport, uint8_t addr = 0)
        : Base(transport, addr), configuredAddress(addr) {}
    void begin() { initialized = false; Base::begin(); }
    void poll() {
        if (!Base::ready()) return;
        switch (Base::state) {
        case 0:
            if (!initialized) {
                Base::address = configuredAddress ? configuredAddress : 0x44;
                if (!Base::command16(0x3093)) { // stop periodic acquisition
                    if (configuredAddress) { bad(); return; }
                    Base::address = 0x45;
                    if (!Base::command16(0x3093)) { bad(); return; }
                }
                Base::state = 1;
                Base::at = Transport::now() + 1;
                return;
            }
            if (!Base::command16(0x2400)) { bad(); return; }
            Base::state = 3;
            // Conservative conversion wait for both fitted manufacturers.
            Base::at = Transport::now() + 30;
            return;
        case 1:
            if (!Base::command16(0x30a2)) { bad(); return; }
            Base::state = 2;
            Base::at = Transport::now() + 2;
            return;
        case 2:
            initialized = true;
            Base::state = 0;
            return;
        case 3: {
            uint8_t bytes[6];
            if (!Base::io.read(Base::address, bytes, 6) || !crcPair(bytes)) { bad(); return; }
            // Commit both fields only after both CRCs have passed.
            temperature = atm::temperature(word(bytes));
            humidity = rounded(10000 * int32_t(word(bytes + 3)), 65535);
            Base::success();
            return;
        }
        }
    }
};

template<class Transport> class Barometer : public Sensor<Transport> {
    using Base = Sensor<Transport>;
    bool initialized = false;
    void bad() { initialized = false; Base::failure(); }
    // 0: waiting; 1: ready; -1: transaction/configuration/deadline error.
    int status(uint8_t required) {
        uint8_t flags;
        if (!Base::reply(0x8d, &flags, 1) || (flags & 0x80) || Base::expired()) return -1;
        if ((flags & required) == required) return 1;
        Base::at = Transport::now() + 2;
        return 0;
    }
public:
    int32_t temperature = 0, pressure = 0;
    explicit Barometer(Transport& transport) : Base(transport, 0x76) {}
    void begin() { initialized = false; Base::begin(); }
    void poll() {
        if (!Base::ready()) return;
        switch (Base::state) {
        case 0:
            if (!initialized) {
                if (!Base::command(0x06)) { bad(); return; }
                Base::state = 1;
                Base::at = Transport::now() + 2;
                Base::limit = Transport::now() + 22;
            } else {
                Base::state = 2;
                Base::limit = Transport::now() + 20;
            }
            return;
        case 1: {
            const int result = status(0x40);
            if (result < 0) { bad(); return; }
            if (!result) return;
            const uint8_t para[] = {0xcf, 0x80}, interrupt[] = {0xcc, 0x00}, ready[] = {0xcb, 0x30};
            uint8_t check;
            if (!Base::io.write(Base::address, para, 2) ||
                !Base::io.write(Base::address, interrupt, 2) ||
                !Base::io.write(Base::address, ready, 2) ||
                !Base::reply(0x8f, &check, 1) || !(check & 0x80)) { bad(); return; }
            initialized = true;
            Base::state = 2;
            Base::limit = Transport::now() + 20;
            return;
        }
        case 2: {
            const int result = status(0x40);
            if (result < 0) { bad(); return; }
            if (!result) return;
            if (!Base::command(0x40)) { bad(); return; } // combined OSR4096
            Base::state = 3;
            Base::at = Transport::now() + 160;
            Base::limit = Transport::now() + 200;
            return;
        }
        case 3: {
            const int result = status(0x70);
            if (result < 0) { bad(); return; }
            if (!result) return;
            uint8_t t[3], p[3];
            if (!Base::reply(0x32, t, 3) || !Base::reply(0x30, p, 3)) { bad(); return; }
            temperature = signed20(t);
            pressure = int32_t(low20(p)) * 100; // raw count is 1 Pa
            Base::success();
            return;
        }
        }
    }
};

template<class Transport> class GaugePressure : public Sensor<Transport> {
    using Base = Sensor<Transport>;
public:
    int32_t temperature = 0, pressure = 0;
    explicit GaugePressure(Transport& transport) : Base(transport, 0x6d) {}
    void poll() {
        if (!Base::ready()) return;
        if (!Base::state) {
            const uint8_t bytes[] = {0x30, 0x0a};
            if (!Base::io.write(Base::address, bytes, 2)) { Base::failure(); return; }
            Base::state = 1;
            Base::at = Transport::now() + 20;
            Base::limit = Transport::now() + 50;
            return;
        }
        uint8_t command;
        if (!Base::reply(0x30, &command, 1) || Base::expired()) { Base::failure(); return; }
        if (command & 0x08) { Base::at = Transport::now() + 1; return; }
        uint8_t bytes[5];
        // Old 0x6d protocol: individual register reads, no assumed auto-increment.
        for (uint8_t i = 0; i < 5; ++i)
            if (!Base::reply(0x06 + i, bytes + i, 1)) { Base::failure(); return; }
        pressure = rounded(signed24(bytes) * 100, 128); // 40 kPa part, K=128
        const int32_t t = word(bytes + 3);
        temperature = rounded((t >= 32768 ? t - 65536 : t) * 100, 256);
        Base::success();
    }
};

template<class Transport> class CarbonDioxide : public Sensor<Transport> {
    using Base = Sensor<Transport>;
    uint32_t lastSample = 0;
    void bad() {
        Base::failure();
        // Restart periodic acquisition after a transport/CRC/timeout failure.
    }
public:
    int32_t temperature = 0, co2 = 0;
    explicit CarbonDioxide(Transport& transport) : Base(transport, 0x62) {}
    void poll() {
        if (!Base::ready()) return;
        switch (Base::state) {
        case 0:
            if (!Base::command16(0x3f86)) { bad(); return; }
            Base::state = 1;
            Base::at = Transport::now() + 500;
            return;
        case 1:
            if (!Base::command16(0x21b1)) { bad(); return; }
            lastSample = Transport::now();
            Base::state = 2;
            Base::at = Transport::now() + 5000;
            return;
        case 2:
            if (!Base::command16(0xe4b8)) { bad(); return; }
            Base::state = 3;
            Base::at = Transport::now() + 1;
            return;
        case 3: {
            uint8_t bytes[3];
            if (!Base::io.read(Base::address, bytes, 3) || crc8(bytes, 2) != bytes[2]) { bad(); return; }
            if (!(word(bytes) & 0x07ff)) {
                if (uint32_t(Transport::now() - lastSample) >= 15000) { bad(); return; }
                Base::state = 2;
                Base::at = Transport::now() + 1000;
                return;
            }
            if (!Base::command16(0xec05)) { bad(); return; }
            Base::state = 4;
            Base::at = Transport::now() + 1;
            return;
        }
        case 4: {
            uint8_t bytes[6];
            // End after the temperature CRC, as in the donor. RH is not used.
            if (!Base::io.read(Base::address, bytes, 6) || !crcPair(bytes)) { bad(); return; }
            co2 = int32_t(word(bytes)) * 100; // hundredths of ppm
            temperature = atm::temperature(word(bytes + 3));
            lastSample = Transport::now();
            Base::success();
            Base::state = 2; // keep the sensor's periodic measurement running
            return;
        }
        }
    }
};
}
