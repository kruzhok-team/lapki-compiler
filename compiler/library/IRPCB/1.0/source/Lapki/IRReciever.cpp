#include "IRReciever.h"

#include "Serial.h"
// #include "ir.h"
#include "File.h"
#include "kl_fs_utils.h"
#include "usb_msdcdc.h"
#include "yartos.h"

IRpkg IRReciever::pkg;
bool IRReciever::isUpdated_ = 0;
uint32_t IRReciever::lastUpdate = Sys::GetSysTime();

extern UsbMsdCdc usb_msd_cdc;

void IRReciever::savePkg() {
    if (usb_msd_cdc.IsActive()) {
        Serial::Printf("Timestamp: %u, Pkg: %s\r\n", Sys::GetSysTime(), pkg);
    } else {
        // XXX
        // File::Printf(fileName, "Timestamp: %u, Pkg: %s\r\n",
        // Sys::GetSysTime(), pkg);
    }
}

void IRReciever::printPkg() {
    Serial::Printf("Timestamp: %u, Pkg: %s\r\n", Sys::GetSysTime(), pkg);
}

void IRReciever::update(uint8_t bits_count, uint16_t word) {
    IRReciever::pkg.set(bits_count, word);
    IRReciever::isUpdated_ = 1;
}

bool IRReciever::isUpdated() {
    bool isUpd = isUpdated_;
    isUpdated_ = 0;
    return isUpd;
}