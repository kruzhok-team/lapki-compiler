extern "C" {
    GPIO_TypeDef* ir_port = GPIOE;
    uint8_t ir_pin = 12;
    
    void ir_off() {
        setPin_PP(ir_port, ir_pin, OFF);
    }

    void ir_on(){
        setPin_PP(ir_port, ir_pin, ON);
    }

    void ir_init() {
        RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
        initPin_PP(ir_port,ir_pin);
        ir_on();
    }

}