#pragma once

#include "modem.c"

class IR {
public:
    IR(){
        ir_modem_init();
        init_modem();
    }

    void sendByte(int byte) {
        ir_tx_send_byte(byte);
    }
    
    void off() {
        ir_off();
    }

    void on(){
        ir_on();
    }
};
