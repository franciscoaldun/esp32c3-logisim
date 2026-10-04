/* Arranque de la aplicacion (se ejecuta desde la flash en 0x42000008). */
    .section .text.entry, "ax"
    .globl _entry
_entry:
    .option push
    .option norelax
    la   gp, __global_pointer$
    .option pop
    la   sp, __stack_top
    /* copiar .data desde la flash a la SRAM */
    la   a0, _data_start
    la   a1, _data_lma
    la   a2, _data_end
1:  bgeu a0, a2, 2f
    lw   t0, 0(a1)
    sw   t0, 0(a0)
    addi a0, a0, 4
    addi a1, a1, 4
    j    1b
2:  /* poner .bss en cero */
    la   a0, _bss_start
    la   a1, _bss_end
3:  bgeu a0, a1, 4f
    sw   zero, 0(a0)
    addi a0, a0, 4
    j    3b
4:  la   t0, _vector_table
    ori  t0, t0, 1
    csrw mtvec, t0
    call sdk_init
    /* constructores de objetos globales (C++) */
    la   s0, __init_array_start
    la   s1, __init_array_end
7:  bgeu s0, s1, 8f
    lw   t0, 0(s0)
    jalr t0
    addi s0, s0, 4
    j    7b
8:  call main
    /* main termino: con DBG=1 la instruccion ebreak detiene la CPU (LED CPU_DETENIDA) */
    ebreak
5:  wfi
    j    5b

/* Tabla de vectores: debe estar alineada a 256 bytes (la CPU ignora mtvec[7:0]).
 * Entrada 0: excepciones. Entradas 1..31: interrupciones de CPU 1..31. */
    .section .text.vectors, "ax"
    .balign 256
    .globl _vector_table
_vector_table:
    .option push
    .option norvc
    j    _panic_entry
    .rept 31
    j    _irq_entry
    .endr
    .option pop

_irq_entry:
    addi sp, sp, -64
    sw   ra, 0(sp)
    sw   t0, 4(sp)
    sw   t1, 8(sp)
    sw   t2, 12(sp)
    sw   a0, 16(sp)
    sw   a1, 20(sp)
    sw   a2, 24(sp)
    sw   a3, 28(sp)
    sw   a4, 32(sp)
    sw   a5, 36(sp)
    sw   a6, 40(sp)
    sw   a7, 44(sp)
    sw   t3, 48(sp)
    sw   t4, 52(sp)
    sw   t5, 56(sp)
    sw   t6, 60(sp)
    csrr a0, mcause
    call sdk_irq_dispatch
    lw   ra, 0(sp)
    lw   t0, 4(sp)
    lw   t1, 8(sp)
    lw   t2, 12(sp)
    lw   a0, 16(sp)
    lw   a1, 20(sp)
    lw   a2, 24(sp)
    lw   a3, 28(sp)
    lw   a4, 32(sp)
    lw   a5, 36(sp)
    lw   a6, 40(sp)
    lw   a7, 44(sp)
    lw   t3, 48(sp)
    lw   t4, 52(sp)
    lw   t5, 56(sp)
    lw   t6, 60(sp)
    addi sp, sp, 64
    mret

_panic_entry:
    addi sp, sp, -128
    sw   x1, 4(sp)
    sw   x3, 12(sp)
    sw   x4, 16(sp)
    sw   x5, 20(sp)
    sw   x6, 24(sp)
    sw   x7, 28(sp)
    sw   x8, 32(sp)
    sw   x9, 36(sp)
    sw   x10, 40(sp)
    sw   x11, 44(sp)
    sw   x12, 48(sp)
    sw   x13, 52(sp)
    sw   x14, 56(sp)
    sw   x15, 60(sp)
    sw   x16, 64(sp)
    sw   x17, 68(sp)
    sw   x18, 72(sp)
    sw   x19, 76(sp)
    sw   x20, 80(sp)
    sw   x21, 84(sp)
    sw   x22, 88(sp)
    sw   x23, 92(sp)
    sw   x24, 96(sp)
    sw   x25, 100(sp)
    sw   x26, 104(sp)
    sw   x27, 108(sp)
    sw   x28, 112(sp)
    sw   x29, 116(sp)
    sw   x30, 120(sp)
    sw   x31, 124(sp)
    addi t0, sp, 128
    sw   t0, 8(sp)          /* sp original */
    mv   a0, sp
    csrr a1, mcause
    csrr a2, mepc
    csrr a3, mtval
    call sdk_panic
6:  j    6b
