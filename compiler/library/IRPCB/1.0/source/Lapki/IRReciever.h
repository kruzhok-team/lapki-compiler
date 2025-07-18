#ifndef IRRECIEVER_H
#define IRRECIEVER_H
#include "IRpkg.h"

namespace IRReciever {

constexpr inline const char* fileName = "irpkgs.txt";
constexpr uint32_t unstablePeriod = 100; //ms

extern bool isUpdated_;

extern IRpkg pkg;

bool isUpdated();

void savePkg();

void printPkg();

// В главный цикл
void update(uint8_t bits_count, uint16_t word);

};  // namespace IRReciever

#endif