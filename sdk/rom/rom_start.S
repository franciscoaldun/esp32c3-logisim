/* ROM de arranque del modelo ESP32-C3 en Logisim (codigo original, NO es la ROM de Espressif).
 * La CPU arranca en 0x40000000 (inicio de la ROM interna). */
    .section .text.init, "ax"
    .globl _rom_start
_rom_start:
    li   sp, 0x3FCDE710          /* misma pila que usa la ROM real (SOC_ROM_STACK_START) */
    la   t0, rom_vectors
    ori  t0, t0, 1               /* modo vectorizado (el ESP32-C3 solo usa este modo) */
    csrw mtvec, t0
    call rom_main
1:  j 1b

    .globl rom_jump_to_app
rom_jump_to_app:                 /* a0 = direccion de entrada de la aplicacion */
    jr   a0

    /* tabla de vectores: entrada 0 = excepciones, 1..31 = interrupciones */
    .balign 256
    .option push
    .option norvc
rom_vectors:
    j rom_trap
    .rept 31
    j rom_trap
    .endr
    .option pop

rom_trap:
    csrr a0, mcause
    csrr a1, mepc
    csrr a2, mtval
    call rom_exception
2:  j 2b
