#include "ir.c"

class IR {
public:
    IR(){
        ir_init();
    }
    
    void off() {
        ir_off();
    }

    void on(){
        ir_on();
    }
};
