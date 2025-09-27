__ram_start__          = ORIGIN(ram);
__ram_size__           = LENGTH(ram);
__ram_end__            = __ram_start__ + __ram_size__;

ENTRY(Reset_Handler)

SECTIONS {
    .data : ALIGN(4) {
        . = ALIGN(4);
        PROVIDE(_textdata = LOADADDR(.data));
        PROVIDE(_data = .);
        _textdata_start = LOADADDR(.data);
        _data_start = .;
        *(.data)
        *(.data.*)
        *(.ramtext)
        *(.fastrun)         /* "RAM-Functions" */
        . = ALIGN(4);
        PROVIDE(_edata = .);
        _data_end = .;
    } > DATA_RAM AT > DATA_RAM_LMA

    .bss (NOLOAD) : ALIGN(4) {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
        PROVIDE(end = .);
    } > BSS_RAM

    /* The default heap uses the (statically) unused part of a RAM section.*/
    .heap (NOLOAD) : {
        . = ALIGN(8);
        __heap_base__ = .;
        . = ORIGIN(HEAP_RAM) + LENGTH(HEAP_RAM);
        __heap_end__ = .;
    } > HEAP_RAM
}
