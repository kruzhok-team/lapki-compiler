#pragma once

#include "IRReciever.h"

// Existing IRpkg and event delivery are shared with the standard receiver.
namespace BlgIrReceiver {
inline IRpkg &pkg = IRReciever::pkg;
void init();
bool isUpdated();
void printPkg();
void printDiagnostics();
}
