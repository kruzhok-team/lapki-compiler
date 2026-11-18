#pragma once

#include "modem.c"
#include "irpcb_modem.c"

class IRPcb {
public:
    IRPcb() {
        ir_modem_init();
        ir_pcb_init();
        init_modem();
    }

    void sendByte(int byte) {
        ir_tx_enqueue(static_cast<uint16_t>(static_cast<uint8_t>(byte)) << 8,
                      8, IR_TX_IRPCB);
    }

    void sendWord(int word) {
        ir_tx_enqueue(static_cast<uint16_t>(word), 16, IR_TX_IRPCB);
    }
};
