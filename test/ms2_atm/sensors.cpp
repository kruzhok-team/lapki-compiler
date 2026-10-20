#include "SensorDrivers.hpp"
#include <assert.h>
#include <deque>
#include <initializer_list>
#include <vector>

struct Bus {
    struct Operation {
        bool read;
        uint8_t address;
        std::vector<uint8_t> bytes;
        bool ok;
    };
    static uint32_t clock;
    static uint32_t now() { return clock; }
    std::deque<Operation> operations;
    void writeAt(uint8_t address, std::initializer_list<uint8_t> bytes, bool ok = true) {
        operations.push_back({false, address, bytes, ok});
    }
    void readAt(uint8_t address, std::initializer_list<uint8_t> bytes, bool ok = true) {
        operations.push_back({true, address, bytes, ok});
    }
    void sample(uint8_t address, uint16_t first, uint16_t second, bool corrupt = false) {
        const uint8_t a[] = {uint8_t(first >> 8), uint8_t(first)};
        const uint8_t b[] = {uint8_t(second >> 8), uint8_t(second)};
        readAt(address, {a[0], a[1], atm::crc8(a, 2), b[0], b[1],
                       uint8_t(atm::crc8(b, 2) ^ uint8_t(corrupt))});
    }
    void status(uint8_t address, uint16_t value) {
        const uint8_t bytes[] = {uint8_t(value >> 8), uint8_t(value)};
        readAt(address, {bytes[0], bytes[1], atm::crc8(bytes, 2)});
    }
    bool write(uint8_t address, const uint8_t* bytes, uint8_t size) {
        assert(!operations.empty());
        Operation op = operations.front(); operations.pop_front();
        assert(!op.read && op.address == address);
        assert(op.bytes == std::vector<uint8_t>(bytes, bytes + size));
        return op.ok;
    }
    bool read(uint8_t address, uint8_t* bytes, uint8_t size) {
        assert(!operations.empty());
        Operation op = operations.front(); operations.pop_front();
        assert(op.read && op.address == address && op.bytes.size() == size);
        for (uint8_t i = 0; i < size; ++i) bytes[i] = op.bytes[i];
        return op.ok;
    }
    ~Bus() { assert(operations.empty()); }
};
uint32_t Bus::clock = 0;

template<class Sensor> void tick(Sensor& sensor, uint32_t time) {
    Bus::clock = time;
    sensor.poll();
}

void humidity() {
    Bus bus;
    atm::HumiditySensor<Bus> sensor(bus);
    Bus::clock = 0;
    sensor.begin();
    assert(!sensor.error && !sensor.valueReceived() && !sensor.readingError());
    bus.writeAt(0x44, {0x30, 0x93}, false);
    bus.writeAt(0x45, {0x30, 0x93});
    tick(sensor, 29); // no transaction before deadline
    tick(sensor, 30);
    bus.writeAt(0x45, {0x30, 0xa2});
    tick(sensor, 31);
    tick(sensor, 33);
    bus.writeAt(0x45, {0x24, 0});
    tick(sensor, 33);
    bus.sample(0x45, 26214, 32768);
    tick(sensor, 62);
    tick(sensor, 63);
    assert(sensor.temperature == 2500 && sensor.humidity == 5000);
    assert(sensor.valueReceived() && !sensor.valueReceived() && !sensor.error);

    bus.writeAt(0x45, {0x24, 0});
    tick(sensor, 1063);
    bus.sample(0x45, 0, 0, true); // bad humidity CRC invalidates the entire pair
    tick(sensor, 1093);
    assert(sensor.error && sensor.readingError() && !sensor.readingError());
    assert(!sensor.valueReceived());
    assert(sensor.temperature == 2500 && sensor.humidity == 5000);

    // Recovery reinitializes just this sensor, discovers 0x44 this time.
    bus.writeAt(0x44, {0x30, 0x93}); tick(sensor, 2093);
    bus.writeAt(0x44, {0x30, 0xa2}); tick(sensor, 2094);
    tick(sensor, 2096);
    bus.writeAt(0x44, {0x24, 0}); tick(sensor, 2096);
    bus.sample(0x44, 0, 65535); tick(sensor, 2126);
    assert(sensor.temperature == -4500 && sensor.humidity == 10000);
    assert(!sensor.error && sensor.valueReceived());
}

void pressure() {
    Bus bus;
    atm::Barometer<Bus> sensor(bus);
    Bus::clock = 0; sensor.begin();
    bus.writeAt(0x76, {0x06}); tick(sensor, 30);
    bus.writeAt(0x76, {0x8d}); bus.readAt(0x76, {0x40});
    bus.writeAt(0x76, {0xcf, 0x80}); bus.writeAt(0x76, {0xcc, 0});
    bus.writeAt(0x76, {0xcb, 0x30});
    bus.writeAt(0x76, {0x8f}); bus.readAt(0x76, {0x80}); tick(sensor, 32);
    bus.writeAt(0x76, {0x8d}); bus.readAt(0x76, {0x40});
    bus.writeAt(0x76, {0x40}); tick(sensor, 32);
    bus.writeAt(0x76, {0x8d}); bus.readAt(0x76, {0x70});
    bus.writeAt(0x76, {0x32}); bus.readAt(0x76, {0, 0x0a, 0x5c});
    bus.writeAt(0x76, {0x30}); bus.readAt(0x76, {1, 0x8a, 0x9e});
    tick(sensor, 191); tick(sensor, 192);
    assert(sensor.temperature == 2652 && sensor.pressure == 10102200);
    assert(sensor.valueReceived());
    tick(sensor, 1192);
    bus.writeAt(0x76, {0x8d}); bus.readAt(0x76, {0x40});
    bus.writeAt(0x76, {0x40}); tick(sensor, 1192);
    bus.writeAt(0x76, {0x8d}); bus.readAt(0x76, {0x40}); tick(sensor, 1392);
    assert(sensor.error && sensor.readingError()); // never publish old results as fresh
    assert(sensor.pressure == 10102200 && !sensor.valueReceived());
    const uint8_t negative[] = {0xff, 0xfc, 0x02};
    assert(atm::signed20(negative) == -1022);
}

void gauge() {
    Bus bus;
    atm::GaugePressure<Bus> sensor(bus);
    Bus::clock = 0; sensor.begin();
    bus.writeAt(0x6d, {0x30, 0x0a}); tick(sensor, 30);
    bus.writeAt(0x6d, {0x30}); bus.readAt(0x6d, {8}); tick(sensor, 50);
    bus.writeAt(0x6d, {0x30}); bus.readAt(0x6d, {0});
    const uint8_t values[] = {0xff, 0xff, 0xff, 0xf6, 0};
    for (uint8_t i = 0; i < 5; ++i) {
        bus.writeAt(0x6d, {uint8_t(6 + i)}); bus.readAt(0x6d, {values[i]});
    }
    tick(sensor, 51);
    assert(sensor.pressure == -1 && sensor.temperature == -1000);
    assert(sensor.valueReceived() && !sensor.error);
    bus.writeAt(0x6d, {0x30, 0x0a}); tick(sensor, 1051);
    bus.writeAt(0x6d, {0x30}); bus.readAt(0x6d, {8}); tick(sensor, 1101);
    assert(sensor.error && sensor.readingError() && !sensor.valueReceived());
}

void carbon() {
    Bus bus;
    atm::CarbonDioxide<Bus> sensor(bus);
    Bus::clock = 0; sensor.begin();
    bus.writeAt(0x62, {0x3f, 0x86}); tick(sensor, 30);
    bus.writeAt(0x62, {0x21, 0xb1}); tick(sensor, 529); tick(sensor, 530);
    bus.writeAt(0x62, {0xe4, 0xb8}); tick(sensor, 5530);
    bus.status(0x62, 1); bus.writeAt(0x62, {0xec, 5}); tick(sensor, 5531);
    bus.sample(0x62, 400, 26214); tick(sensor, 5532);
    assert(sensor.co2 == 40000 && sensor.temperature == 2500 && sensor.valueReceived());
    bus.writeAt(0x62, {0xe4, 0xb8}); tick(sensor, 6532);
    bus.status(0x62, 0); tick(sensor, 6533);
    assert(!sensor.error && !sensor.readingError() && !sensor.valueReceived());
    bus.writeAt(0x62, {0xe4, 0xb8}); tick(sensor, 20532);
    bus.status(0x62, 0); tick(sensor, 20533);
    assert(sensor.error && sensor.readingError() && sensor.co2 == 40000);
    bus.writeAt(0x62, {0x3f, 0x86}); tick(sensor, 21533); // recovery starts with STOP
}

void wraparound() {
    Bus bus;
    atm::HumiditySensor<Bus> sensor(bus, 0x44);
    Bus::clock = 0xfffffff0u; sensor.begin(); // deadline wraps to 14
    tick(sensor, 13);
    bus.writeAt(0x44, {0x30, 0x93}, false); tick(sensor, 14);
    assert(sensor.error && sensor.readingError());
}

void independentSensors() {
    Bus first, second;
    atm::HumiditySensor<Bus> s4(first, 0x44), s5(second, 0x45);
    Bus::clock = 0; s4.begin(); s5.begin();
    first.writeAt(0x44, {0x30, 0x93}, false); tick(s4, 30);
    second.writeAt(0x45, {0x30, 0x93}); tick(s5, 30);
    assert(s4.error && !s5.error && !s5.readingError());
}

void partialGaugeRead() {
    Bus bus;
    atm::GaugePressure<Bus> sensor(bus);
    Bus::clock = 0; sensor.begin();
    // Seed an earlier successful pair; a later partial read must preserve it.
    sensor.pressure = 12300; sensor.temperature = 2500;
    bus.writeAt(0x6d, {0x30, 0x0a}); tick(sensor, 30);
    bus.writeAt(0x6d, {0x30}); bus.readAt(0x6d, {0});
    bus.writeAt(0x6d, {0x06}); bus.readAt(0x6d, {0x7f});
    bus.writeAt(0x6d, {0x07}); bus.readAt(0x6d, {0}, false);
    tick(sensor, 50);
    assert(sensor.error && sensor.readingError() && !sensor.valueReceived());
    assert(sensor.pressure == 12300 && sensor.temperature == 2500);
}

int main() {
    const uint8_t example[] = {0xbe, 0xef};
    assert(atm::crc8(example, 2) == 0x92);
    humidity(); pressure(); gauge(); carbon(); wraparound(); independentSensors(); partialGaugeRead();
}
