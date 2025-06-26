#ifndef IRRECIEVER_H
#define IRRECIEVER_H
#include "IRpkg.h"

namespace IRReciever {

constexpr inline const char* fileName = "irpkgs.txt";

extern IRpkg pkg;

extern bool isUpdated_;

bool isUpdated();

void savePkg();

void printPkg();

// В главный цикл
void update(uint8_t bits_count, uint16_t word);

};  // namespace IRReciever

#endif