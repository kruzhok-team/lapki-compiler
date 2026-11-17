#include "BlgIrReceiver.h"
#include "ir.h"
#include "Serial.h"
#include "Settings.h"

void BlgIrReceiver::init() {
    IRReciever::isUpdated_ = false;
    pkg.set(0, 0);
    IRRcvr::SetMode(CYBERBEAR_MODE);
}

bool BlgIrReceiver::isUpdated() { return IRReciever::isUpdated(); }
void BlgIrReceiver::printPkg() { IRReciever::printPkg(); }

void BlgIrReceiver::printDiagnostics() {
    auto d = IRRcvr::GetDiagnostics();
    Serial::Printf("IR RX mode=%u samples=%u edges=%u fast=%u slow=%u headers=%u packets=%u rejected=%u\r\n",
        static_cast<uint32_t>(settings.rx_mode), d.samples, d.edges, d.fast_intervals,
        d.slow_intervals, d.headers, d.packets, d.rejected);
}
