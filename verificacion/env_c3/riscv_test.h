// Entorno de riscv-tests para el ESP32-C3 en Logisim.
// La imagen arranca por la ROM del modelo (salta a 0x42000008, ver entry.S). Al terminar escribe
// por la UART0 (0x60000000):
//   PASS
//   F <caso>                         si falla una verificacion (caso = TESTNUM)
//   T <caso> <mcause> <mepc>         si ocurre una excepcion inesperada
#ifndef _ENV_C3_LOGISIM_H
#define _ENV_C3_LOGISIM_H

#include "../riscv-tests/env/encoding.h"

#define RVTEST_RV64U .macro init; .endm
#define RVTEST_RV32U .macro init; .endm

#define TESTNUM gp

#define UART0_FIFO 0x60000000

// imprime t2 en hexadecimal (8 cifras) por la UART; usa t3..t5, t0 = UART0_FIFO
#define PRINT_HEX(lbl)                                                  \
        li t5, 28;                                                      \
lbl##1: srl t3, t2, t5; andi t3, t3, 15;                                \
        li t4, 10; blt t3, t4, lbl##2; addi t3, t3, 'a'-10-'0';         \
lbl##2: addi t3, t3, '0'; sw t3, 0(t0);                                 \
        addi t5, t5, -4; bgez t5, lbl##1;

#define RVTEST_CODE_BEGIN                                               \
        .section .text.init;                                            \
        .globl _start;                                                  \
_start:                                                                 \
        /* copiar .data de la flash (vista de datos) a la SRAM */       \
        la t0, _data_start;                                             \
        la t1, _data_lma;                                               \
        la t2, _data_end;                                               \
.Lcopia:  bgeu t0, t2, .Lcopiado;                                                \
        lw t3, 0(t1);                                                   \
        sw t3, 0(t0);                                                   \
        addi t0, t0, 4;                                                 \
        addi t1, t1, 4;                                                 \
        j .Lcopia;                                                         \
.Lcopiado: la t0, trap_vector;                                             \
        csrw mtvec, t0;                                                 \
        li x1, 0; li x2, 0; li x3, 0; li x4, 0; li x5, 0; li x6, 0;     \
        li x7, 0; li x8, 0; li x9, 0; li x10, 0; li x11, 0; li x12, 0;  \
        li x13, 0; li x14, 0; li x15, 0; li x16, 0; li x17, 0;          \
        li x18, 0; li x19, 0; li x20, 0; li x21, 0; li x22, 0;          \
        li x23, 0; li x24, 0; li x25, 0; li x26, 0; li x27, 0;          \
        li x28, 0; li x29, 0; li x30, 0; li x31, 0;                     \
        init;                                                           \
        j test_start;                                                   \
        /* la CPU ignora mtvec[7:0]: la tabla va alineada a 256 */      \
        .balign 256;                                                    \
trap_vector:                                                            \
        csrr a1, mcause;                                                \
        csrr a2, mepc;                                                  \
        li a0, 'T';                                                     \
        j report_trap;                                                  \
test_start:

#define RVTEST_CODE_END                                                 \
report_pass:                                                            \
        li t0, UART0_FIFO;                                              \
        li t1, 'P'; sw t1, 0(t0);                                       \
        li t1, 'A'; sw t1, 0(t0);                                       \
        li t1, 'S'; sw t1, 0(t0);                                       \
        li t1, 'S'; sw t1, 0(t0);                                       \
        li t1, '\n'; sw t1, 0(t0);                                      \
.Lfin_pass: j .Lfin_pass;                                                           \
report_fail:                                                            \
        li t0, UART0_FIFO;                                              \
        li t1, 'F'; sw t1, 0(t0);                                       \
        li t1, ' '; sw t1, 0(t0);                                       \
        mv t2, TESTNUM;                                                 \
        PRINT_HEX(.Lpf)                                                 \
        li t1, '\n'; sw t1, 0(t0);                                      \
.Lfin_fail: j .Lfin_fail;                                                           \
report_trap:                                                            \
        li t0, UART0_FIFO;                                              \
        sw a0, 0(t0);                                                   \
        li t1, ' '; sw t1, 0(t0);                                       \
        mv t2, TESTNUM;                                                 \
        PRINT_HEX(.Lpt)                                                 \
        li t1, ' '; sw t1, 0(t0);                                       \
        mv t2, a1;                                                      \
        PRINT_HEX(.Lpc)                                                 \
        li t1, ' '; sw t1, 0(t0);                                       \
        mv t2, a2;                                                      \
        PRINT_HEX(.Lpe)                                                 \
        li t1, '\n'; sw t1, 0(t0);                                      \
.Lfin_trap: j .Lfin_trap;

#define RVTEST_PASS  fence; j report_pass;
#define RVTEST_FAIL  fence; j report_fail;

#define EXTRA_DATA
#define RVTEST_DATA_BEGIN EXTRA_DATA .align 4; .global begin_signature; begin_signature:
#define RVTEST_DATA_END .align 4; .global end_signature; end_signature:

#endif
