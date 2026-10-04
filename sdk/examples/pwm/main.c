/* PWM con el periferico LEDC: tres canales con distinto ciclo de trabajo y uno que "respira"
 * (variacion automatica del ciclo de trabajo hecha por el hardware), en GPIO18.
 * Los LEDs son de encendido/apagado: el ciclo de trabajo se ve como la fraccion del tiempo encendido. */
#include "sdk.h"

#define BITS     6          /* resolucion: el contador va de 0 a 63 */
#define DIVISOR  8          /* un paso del contador cada 8 ciclos: periodo = 64 * 8 = 512 ciclos */

static volatile int vueltas;

static void al_terminar_variacion(void *arg)
{
    REG_WRITE(LEDC_INT_CLR_REG, LEDC_DUTY_CHNG_END_INT(3));
    vueltas++;
    /* cambia de sentido: sube 64 pasos de 1 y luego baja */
    sdk_ledc_fade(3, vueltas & 1 ? 0 : 1, 63, 1, 1);
}

int main(void)
{
    printf("\nPWM con LEDC: periodo = %d ciclos\n", (1 << BITS) * DIVISOR);
    sdk_ledc_timer(0, BITS, DIVISOR);
    sdk_ledc_channel(0, 0, 0, 16);       /* GPIO0: 16/64 = 25 % */
    sdk_ledc_channel(1, 0, 1, 32);       /* GPIO1: 50 % */
    sdk_ledc_channel(2, 0, 3, 48);       /* GPIO3: 75 % */
    sdk_ledc_channel(3, 0, 18, 0);       /* GPIO18: variacion automatica */
    printf("GPIO0 = 25%%, GPIO1 = 50%%, GPIO3 = 75%%, GPIO18 respira\n");
    REG_WRITE(LEDC_INT_CLR_REG, 0xFFFF);
    REG_SET_BIT(LEDC_INT_ENA_REG, LEDC_DUTY_CHNG_END_INT(3));
    intr_attach(ETS_LEDC_INTR_SOURCE, 1, 1, al_terminar_variacion, 0);
    intr_global_enable();
    sdk_ledc_fade(3, 1, 63, 1, 1);              /* sube de 0 a 63 en 63 pasos, uno por periodo */
    int ultima = -1;
    for (;;) {
        uint32_t duty = REG_READ(LEDC_CHn_DUTY_R_REG(3)) >> 4;
        if ((int)duty / 8 != ultima) {
            ultima = duty / 8;
            printf("duty GPIO18 = %2d/64 ", duty);
            for (int i = 0; i < 8; i++) putchar(i <= ultima ? '#' : '.');
            putchar('\n');
        }
    }
}
