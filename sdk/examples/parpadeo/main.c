/* Parpadeo: el LED rojo (GPIO8) parpadea. Mientras mantienes BOOT (GPIO9) parpadea mas rapido.
 * BOTON_1 (GPIO2) enciende el LED de GPIO3 mientras lo presionas. */
#include "sdk.h"

int main(void)
{
    gpio_output(8);             /* LED rojo de la placa */
    gpio_output(3);
    gpio_input(9, 1);           /* BOOT, con pull-up: vale 0 cuando se presiona */
    gpio_input(2, 0);           /* BOTON_1 pone el pin en 1 */
    printf("\nParpadeo en GPIO8 (LED rojo).\n");
    printf("Manten BOOT para ir mas rapido; BOTON_1 enciende el LED de GPIO3.\n");
    int rapido = 0;
    for (;;) {
        gpio_toggle(8);
        int r = !gpio_get(9);
        if (r != rapido) {
            rapido = r;
            printf(r ? "BOOT presionado: rapido\n" : "BOOT suelto: lento\n");
        }
        uint32_t espera = r ? 150 : 600;          /* ciclos de reloj (el SYSTIMER cuenta 1 por ciclo) */
        uint32_t t0 = (uint32_t)systimer_ticks();
        while ((uint32_t)systimer_ticks() - t0 < espera) {
            gpio_set(3, gpio_get(2));
        }
    }
}
