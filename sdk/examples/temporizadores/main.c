/* Temporizadores y perro guardian (watchdog):
 *  - TIMG0 T0 genera una alarma periodica (interrupcion) que cambia el LED de GPIO4.
 *  - Una interrupcion por software (FROM_CPU0) se dispara escribiendo un registro.
 *  - El watchdog MWDT de TIMG0 se alimenta 5 veces y luego no: al vencer reinicia el chip.
 *    El registro RTC_CNTL_STORE0 sobrevive al reinicio y cuenta los arranques. */
#include "sdk.h"

static volatile int alarmas;

static void al_temporizador(void *arg)
{
    REG_WRITE(TIMG_INT_CLR_TIMERS_REG(0), BIT(0));
    REG_SET_BIT(TIMG_T0CONFIG_REG(0), TIMG_T0_ALARM_EN);       /* la alarma se desactiva sola al dispararse */
    alarmas++;
    gpio_toggle(4);
}

static void al_software(void *arg)
{
    REG_WRITE(SYSTEM_CPU_INTR_FROM_CPU_n_REG(0), 0);
    printf("  -> interrupcion por software atendida\n");
}

static void alimentar_perro(void)
{
    REG_WRITE(TIMG_WDTWPROTECT_REG(0), TIMG_WDT_WKEY_VALUE);
    REG_WRITE(TIMG_WDTFEED_REG(0), 1);
    REG_WRITE(TIMG_WDTWPROTECT_REG(0), 0);
}

int main(void)
{
    uint32_t arranques = REG_READ(RTC_CNTL_STORE0_REG) + 1;
    REG_WRITE(RTC_CNTL_STORE0_REG, arranques);
    printf("\nTemporizadores - arranque #%d, reset: %s\n", arranques, reset_reason_name(reset_reason()));
    gpio_output(4);

    intr_attach(ETS_FROM_CPU_INTR0_SOURCE, 5, 1, al_software, 0);
    intr_attach(ETS_TG0_T0_LEVEL_INTR_SOURCE, 1, 1, al_temporizador, 0);
    intr_global_enable();
    printf("Disparo una interrupcion por software:\n");
    REG_WRITE(SYSTEM_CPU_INTR_FROM_CPU_n_REG(0), 1);

    /* TIMG0 T0: divisor 2, cuenta hacia arriba, recarga automatica, alarma cada 400 cuentas */
    REG_WRITE(TIMG_T0CONFIG_REG(0), TIMG_T0_INCREASE | TIMG_T0_AUTORELOAD | (2 << TIMG_T0_DIVIDER_S));
    REG_WRITE(TIMG_T0LOADLO_REG(0), 0);
    REG_WRITE(TIMG_T0LOADHI_REG(0), 0);
    REG_WRITE(TIMG_T0LOAD_REG(0), 1);
    REG_WRITE(TIMG_T0ALARMLO_REG(0), 400);
    REG_WRITE(TIMG_T0ALARMHI_REG(0), 0);
    REG_WRITE(TIMG_INT_CLR_TIMERS_REG(0), BIT(0));
    REG_SET_BIT(TIMG_INT_ENA_TIMERS_REG(0), BIT(0));
    REG_SET_BIT(TIMG_T0CONFIG_REG(0), TIMG_T0_EN | TIMG_T0_ALARM_EN);

    if (arranques > 1) {
        printf("El watchdog ya reinicio el chip. Solo corre el temporizador.\n");
        int ultimo = 0;
        for (;;) {
            if (alarmas != ultimo && alarmas % 5 == 0) {
                ultimo = alarmas;
                printf("alarmas del temporizador: %d\n", alarmas);
            }
        }
    }

    /* MWDT de TIMG0: etapa 0 -> reinicio del sistema tras 8000 ciclos sin alimentarlo */
    REG_WRITE(TIMG_WDTWPROTECT_REG(0), TIMG_WDT_WKEY_VALUE);
    REG_WRITE(TIMG_WDTCONFIG1_REG(0), 1 << 16);                     /* preescalador = 1 */
    REG_WRITE(TIMG_WDTCONFIG2_REG(0), 8000);                        /* duracion de la etapa 0 */
    REG_WRITE(TIMG_WDTCONFIG0_REG(0), TIMG_WDT_EN | (3 << TIMG_WDT_STG0_S));
    REG_WRITE(TIMG_WDTFEED_REG(0), 1);
    REG_WRITE(TIMG_WDTWPROTECT_REG(0), 0);

    for (int i = 1; i <= 5; i++) {
        delay_ticks(3000);
        alimentar_perro();
        printf("alimento al perro (%d/5), alarmas=%d\n", i, alarmas);
    }
    printf("Ya no lo alimento: el watchdog va a reiniciar el chip...\n");
    for (;;) { }
}
