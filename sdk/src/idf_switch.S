/* Cambio de contexto del FreeRTOS cooperativo del modelo.
 * void rtos_switch(uint32_t *save_sp, uint32_t new_sp)
 * Guarda en la pila actual los registros que una funcion debe preservar (ra, s0..s11), anota sp,
 * cambia a la pila de la otra tarea y recupera los suyos. Al hacer 'ret' se continua en la otra tarea. */
    .section .text.rtos_switch, "ax"
    .globl rtos_switch
rtos_switch:
    addi sp, sp, -64
    sw   ra, 0(sp)
    sw   s0, 4(sp)
    sw   s1, 8(sp)
    sw   s2, 12(sp)
    sw   s3, 16(sp)
    sw   s4, 20(sp)
    sw   s5, 24(sp)
    sw   s6, 28(sp)
    sw   s7, 32(sp)
    sw   s8, 36(sp)
    sw   s9, 40(sp)
    sw   s10, 44(sp)
    sw   s11, 48(sp)
    sw   sp, 0(a0)
    mv   sp, a1
    lw   ra, 0(sp)
    lw   s0, 4(sp)
    lw   s1, 8(sp)
    lw   s2, 12(sp)
    lw   s3, 16(sp)
    lw   s4, 20(sp)
    lw   s5, 24(sp)
    lw   s6, 28(sp)
    lw   s7, 32(sp)
    lw   s8, 36(sp)
    lw   s9, 40(sp)
    lw   s10, 44(sp)
    lw   s11, 48(sp)
    addi sp, sp, 64
    ret

/* primera ejecucion de una tarea nueva: s0 = funcion, s1 = parametro */
    .globl rtos_task_start
rtos_task_start:
    mv   a0, s1
    jalr s0
    call rtos_task_exit
1:  j    1b
