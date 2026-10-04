/* Script de enlace para aplicaciones "direct boot" del ESP32-C3 (modelo Logisim y chip real).
 * La flash se ve en 0x42000000 (codigo) y en 0x3C000000 (datos de solo lectura) con el mismo
 * desplazamiento; la ROM salta a 0x42000008 tras comprobar la "palabra magica" 0xaedb041d x2. */
OUTPUT_ARCH("riscv")
ENTRY(_entry)
MEMORY
{
    irom (rx) : ORIGIN = 0x42000000, LENGTH = 1M
    drom (r)  : ORIGIN = 0x3C000000, LENGTH = 1M
    dram (rw) : ORIGIN = 0x3FC80000, LENGTH = 0x50000
}
__stack_top = ORIGIN(dram) + LENGTH(dram);
SECTIONS
{
    .header : AT(0)
    {
        LONG(0xaedb041d)
        LONG(0xaedb041d)
    } > irom
    .text ORIGIN(irom) + 8 :
    {
        KEEP(*(.text.entry))
        . = ALIGN(256);
        KEEP(*(.text.vectors))
        *(.text .text.*)
        . = ALIGN(4);
    } > irom
    _irom_size = . - ORIGIN(irom);
    .rodata ORIGIN(drom) + _irom_size : AT(_irom_size)
    {
        *(.rodata .rodata.* .srodata .srodata.*)
        . = ALIGN(4);
        /* constructores de objetos globales (C++) */
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array .ctors))
        __init_array_end = .;
        . = ALIGN(4);
    } > drom
    _drom_end_off = LOADADDR(.rodata) + SIZEOF(.rodata);
    .data : AT(_drom_end_off)
    {
        _data_start = .;
        *(.data .data.* .sdata .sdata.*)
        . = ALIGN(4);
        _data_end = .;
    } > dram
    _data_lma = ORIGIN(drom) + LOADADDR(.data);
    .bss (NOLOAD) :
    {
        _bss_start = .;
        *(.bss .bss.* .sbss .sbss.* COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > dram
    __global_pointer$ = _data_start + 0x800;
    /* memoria dinamica (malloc, pilas de las tareas): desde el final de .bss hasta 16 KB antes del tope */
    _heap_start = _bss_end;
    _heap_end = __stack_top - 0x4000;
    /DISCARD/ : { *(.comment .riscv.attributes .note .note.* .eh_frame .eh_frame_hdr .fini_array .fini_array.*) }
}
