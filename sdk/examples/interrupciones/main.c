/* Interrupciones: tres fuentes conectadas por la matriz de interrupciones a tres lineas de la CPU.
 *  - SYSTIMER (alarma periodica)  -> linea 1: cambia el LED de GPIO3 y cuenta "ticks"
 *  - GPIO9 (boton BOOT, flanco de bajada) -> linea 2: avisa por la consola
 *  - UART0 (llega un caracter por el teclado) -> linea 3: lo repite en mayusculas */
#include "sdk.h"

static volatile int ticks;

static void al_systimer(void *arg)
{
    REG_WRITE(SYSTIMER_INT_CLR_REG, BIT(0));        /* borra la peticion */
    ticks++;
    gpio_toggle(3);
}

static void al_boton(void *arg)
{
    uint32_t st = REG_READ(GPIO_STATUS_REG);
    REG_WRITE(GPIO_STATUS_W1TC_REG, st);
    printf("[GPIO] boton BOOT presionado (GPIO_STATUS=0x%x)\n", st);
}

static void al_uart(void *arg)
{
    int c;
    while ((c = uart_getc(0)) >= 0) {
        if (c >= 'a' && c <= 'z') c -= 32;
        putchar_(c == '\r' ? '\n' : (char)c);
    }
    REG_WRITE(UART_INT_CLR_REG(0), UART_RXFIFO_FULL_INT | UART_RXFIFO_TOUT_INT);
}

int main(void)
{
    printf("\nEjemplo de interrupciones (SYSTIMER, GPIO9, UART0)\n");
    gpio_output(3);
    gpio_input(9, 1);

    /* SYSTIMER: comparador 0 en modo periodico, cada 2000 ciclos */
    REG_WRITE(SYSTIMER_TARGETn_CONF_REG(0), SYSTIMER_TARGET_PERIOD_MODE | 2000);
    REG_WRITE(SYSTIMER_COMPn_LOAD_REG(0), 1);
    REG_SET_BIT(SYSTIMER_CONF_REG, SYSTIMER_TARGETn_WORK_EN(0));
    REG_WRITE(SYSTIMER_INT_CLR_REG, BIT(0));
    REG_SET_BIT(SYSTIMER_INT_ENA_REG, BIT(0));
    intr_attach(ETS_SYSTIMER_TARGET0_INTR_SOURCE, 1, 1, al_systimer, 0);

    /* GPIO9: interrupcion en el flanco de bajada */
    REG_WRITE(GPIO_PIN_REG(9), (GPIO_INTR_NEGEDGE << GPIO_PIN_INT_TYPE_S) | (1 << GPIO_PIN_INT_ENA_S));
    REG_WRITE(GPIO_STATUS_W1TC_REG, 0xFFFFFFFF);
    intr_attach(ETS_GPIO_INTR_SOURCE, 2, 2, al_boton, 0);

    /* UART0: interrupcion cuando hay al menos 1 byte en la FIFO de recepcion */
    REG_WRITE(UART_CONF1_REG(0), (REG_READ(UART_CONF1_REG(0)) & ~0x1FFu) | 1);
    REG_WRITE(UART_INT_CLR_REG(0), 0xFFFFFFFF);
    REG_WRITE(UART_INT_ENA_REG(0), UART_RXFIFO_FULL_INT);
    intr_attach(ETS_UART0_INTR_SOURCE, 3, 3, al_uart, 0);

    intr_global_enable();
    int ultimo = -1;
    for (;;) {
        __asm__ volatile ("wfi");                   /* duerme hasta la siguiente interrupcion */
        if (ticks != ultimo) {
            ultimo = ticks;
            if (ultimo % 4 == 0) printf("ticks = %d\n", ultimo);
        }
    }
}
