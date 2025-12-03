/* GD32E103xB LD script, (c) Kreyl */

__main_stack_size__     = 0x0400; 
__process_stack_size__  = 0x0400;

MEMORY {
    flash (xr)   : org = 0x08006000, len = 104k
    ram   (xrw)  : org = 0x20000000, len = 32k
}

/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

/* Flash region to be used for exception vectors.*/
REGION_ALIAS("VECTORS_FLASH", flash);
REGION_ALIAS("VECTORS_FLASH_LMA", flash);

/* Flash region to be used for constructors and destructors.*/
REGION_ALIAS("XTORS_FLASH", flash);
REGION_ALIAS("XTORS_FLASH_LMA", flash);

/* Flash region to be used for code text.*/
REGION_ALIAS("TEXT_FLASH", flash);
REGION_ALIAS("TEXT_FLASH_LMA", flash);

/* Flash region to be used for read only data.*/
REGION_ALIAS("RODATA_FLASH", flash);
REGION_ALIAS("RODATA_FLASH_LMA", flash);

/* Flash region to be used for various.*/
REGION_ALIAS("VARIOUS_FLASH", flash);
REGION_ALIAS("VARIOUS_FLASH_LMA", flash);

/* Flash region to be used for RAM(n) initialization data.*/
REGION_ALIAS("RAM_INIT_FLASH_LMA", flash);

/* RAM region to be used for Main stack. This stack accommodates the processing of all exceptions and interrupts.*/
REGION_ALIAS("MAIN_STACK_RAM", ram);

/* RAM region to be used for the process stack. This is the stack used by the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", ram);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram);
REGION_ALIAS("DATA_RAM_LMA", flash);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", ram);

/* Generic rules inclusion.*/
INCLUDE ./rules.ld