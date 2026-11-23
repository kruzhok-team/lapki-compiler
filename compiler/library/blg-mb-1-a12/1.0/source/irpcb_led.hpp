#pragma once

#include "ir_modem.c"

#if CYBERBEAR_IRPCB_ENABLED
#include "modem.c"
#include "irpcb_modem.c"
#endif

class IRPcb {
public:
    IRPcb() {
#if CYBERBEAR_IRPCB_ENABLED
        ir_modem_init();
        ir_pcb_init();
        init_modem();
#endif
    }

    void sendByte(int byte) {
#if CYBERBEAR_IRPCB_ENABLED
        ir_tx_enqueue(static_cast<uint16_t>(static_cast<uint8_t>(byte)) << 8,
                      8, IR_TX_IRPCB);
#else
        (void)byte;
#endif
    }

    void sendWord(int word) {
#if CYBERBEAR_IRPCB_ENABLED
        ir_tx_enqueue(static_cast<uint16_t>(word), 16, IR_TX_IRPCB);
#else
        (void)word;
#endif
    }
};
