MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Vector table */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Code + read-only data */
    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text*)

        *(.rodata)
        *(.rodata*)

        *(.glue_7)
        *(.glue_7t)

        . = ALIGN(4);

        _etext = .;
    } > FLASH

    /* Initialized data */
    .data : AT(_etext)
    {
        . = ALIGN(4);

        _sidata = LOADADDR(.data);

        _sdata = .;

        *(.data)
        *(.data*)

        . = ALIGN(4);

        _edata = .;
    } > RAM

    /* Uninitialized data */
    .bss :
    {
        . = ALIGN(4);

        __bss_start__ = .;
        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
        __bss_end__ = .;
    } > RAM

    /* End of RAM */
    end = .;
    _end = .;

    PROVIDE(end = .);
}
