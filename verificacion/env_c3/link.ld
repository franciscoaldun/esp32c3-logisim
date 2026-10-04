OUTPUT_ARCH("riscv")
ENTRY(_entry)
MEMORY
{
    irom (rx) : ORIGIN = 0x42000000, LENGTH = 1M
    dram (rw) : ORIGIN = 0x3FC80000, LENGTH = 0x50000
}
SECTIONS
{
    .header : AT(0) { LONG(0xaedb041d) LONG(0xaedb041d) } > irom
    .text ORIGIN(irom) + 8 : { KEEP(*(.text.entry)) KEEP(*(.text.init)) *(.text .text.*) *(.text.trap) *(.rodata .rodata.*) . = ALIGN(4); } > irom
    _data_lma = 0x3C000000 + 8 + SIZEOF(.text);
    .data : AT(8 + SIZEOF(.text)) { _data_start = .; *(.data .data.* .sdata .sdata.* .tohost) . = ALIGN(4); _data_end = .; } > dram
    .bss (NOLOAD) : { *(.bss .bss.* .sbss .sbss.*) } > dram
}
