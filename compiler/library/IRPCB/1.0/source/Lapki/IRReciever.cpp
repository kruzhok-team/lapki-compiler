#include "IRReciever.h"

#include "Serial.h"
// #include "ir.h"
#include "File.h"
#include "kl_fs_utils.h"
#include "yartos.h"

IRpkg IRReciever::pkg;
bool IRReciever::isUpdated_ = 0;

void IRReciever::savePkg() {
    File::printVal(fileName, "\n\rTimestamp: ", Sys::GetSysTime());
    File::print(fileName, pkg);
}

void IRReciever::printPkg() { Serial::printVal("PKG: ", pkg); }

// TODO будем ли создавать отдельный поток?
void IRReciever::update(uint8_t bits_count, uint16_t word) {
    IRReciever::pkg .set(bits_count, word);
    IRReciever::isUpdated_ = 1;
}

bool IRReciever::isUpdated() {
    bool isUpd = isUpdated_;
    isUpdated_ = 0;
    return isUpd;
}