#pragma once

// #include "ir.c"
#include "modem.c"

class IR {
public:
    IR(){
        // ir_init();
        ir_modem_init();
        init_modem();
    }

    void sendByte(int byte) {
        tx.bit_queue = byte;
        tx.not_empty = 1;
    }
    
    void off() {
        // ir_off();
    }

    void on(){
        // ir_on();
    }
};
