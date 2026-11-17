#pragma once
#define STANDARD_MODE 0
#define CYBERBEAR_MODE 1
static int deviation = 150;
struct {
    uint8_t tx_mode = STANDARD_MODE;
    uint8_t rx_mode = STANDARD_MODE;
    int *ir_rx_deviation = &deviation;
} settings;
