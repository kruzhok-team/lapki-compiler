#ifndef IRRECIEVER_H
#define IRRECIEVER_H
#include "IRpkg.h"

namespace IRReciever {

constexpr inline const char* fileName = "irpkgs.txt";
constexpr uint32_t unstablePeriod = 39; //ms could be lower(but 20ms is too small)

extern bool isUpdated_;

extern IRpkg pkg;

bool isUpdated();

void savePkg();

void printPkg();

// to main loop
void update(uint8_t bits_count, uint16_t word);

};  // namespace IRReciever

#endif