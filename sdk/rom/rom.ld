OUTPUT_ARCH("riscv")
ENTRY(_rom_start)
MEMORY { rom (rx) : ORIGIN = 0x40000000, LENGTH = 16K }
SECTIONS
{
  .text : { *(.text.init) *(.text .text.*) } > rom
  .rodata : { *(.rodata .rodata.* .srodata .srodata.*) } > rom
  /DISCARD/ : { *(.data .data.* .sdata .sdata.* .bss .bss.* .sbss .sbss.* .comment .riscv.attributes) }
}
