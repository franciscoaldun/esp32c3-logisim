/*
 * sdk.h - Mini SDK para el ESP32-C3 (modelo Logisim). Funciones simples sobre los registros reales.
 * Encima de este SDK hay dos "capas" para escribir como en Arduino (Arduino.h) o como en ESP-IDF
 * (driver/gpio.h, freertos/task.h, ...).
 */
#ifndef SDK_H
#define SDK_H
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "esp32c3.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- tiempo ----
 * El modelo corre a unos cientos de ciclos por segundo (el chip real a 160 millones), asi que el tiempo
 * va "en camara lenta": por defecto 1 milisegundo del programa = SIM_CICLOS_POR_MS ciclos de reloj.
 * (El SYSTIMER del modelo cuenta 1 por ciclo.)  Se puede cambiar al compilar: -DSIM_CICLOS_POR_MS=4 */
#ifndef SIM_CICLOS_POR_MS
#define SIM_CICLOS_POR_MS 1
#endif
uint64_t systimer_ticks(void);           /* ciclos de reloj desde el arranque */
void delay_ticks(uint32_t ticks);
uint32_t sdk_millis(void);
uint64_t sdk_micros(void);
void sdk_delay_ms(uint32_t ms);
void sdk_delay_us(uint32_t us);

/* ---- consola por UART0 (GPIO21 = TX, GPIO20 = RX) ---- */
void uart_putc(int uart, char c);
void uart_puts(int uart, const char *s);
int  uart_getc(int uart);            /* devuelve -1 si no hay datos */
int  uart_getc_wait(int uart);       /* espera un caracter */
int  uart_available(int uart);       /* bytes esperando en la FIFO de recepcion */
/* escribe n bytes de una vez (crlf = 1: cambia \n por \r\n); mucho mas rapido que uart_putc letra por letra */
void uart_write_n(int uart, const char *s, int n, int crlf);
/* nombres de la primera version (siguen funcionando) */
void putchar_(char c);
void puts_(const char *s);
int  printf_(const char *fmt, ...);
int  vprintf_(const char *fmt, va_list ap);

/* ---- GPIO ---- */
void gpio_output(int pin);
void gpio_input(int pin, int pullup);
void gpio_input_pull(int pin, int pullup, int pulldown);
void gpio_set(int pin, int value);
void gpio_toggle(int pin);
int  gpio_get(int pin);
/* interrupcion por pin: type = GPIO_INTR_POSEDGE, NEGEDGE, ANYEDGE, LOW_LEVEL, HIGH_LEVEL (0 = quitar) */
void gpio_pin_isr(int pin, int type, void (*handler)(void *arg), void *arg);

/* ---- matriz GPIO / IO MUX ---- */
void gpio_matrix_out(int pin, int signal);          /* saca la senal 'signal' de un periferico por 'pin' */
void gpio_iomux_func(int pin, int func);            /* elige la funcion del IO MUX (1 = GPIO, 2 = FSPI...) */

/* ---- relojes de los perifericos (SYSTEM_PERIP_CLK_EN0) ---- */
void periph_enable(uint32_t clk_en_bit);            /* enciende el reloj y saca del reset */

/* ---- LEDC (PWM) ---- */
void sdk_ledc_timer(int timer, int bits, int divisor);          /* periodo = 2^bits * divisor ciclos */
void sdk_ledc_channel(int ch, int timer, int pin, uint32_t duty);
void sdk_ledc_duty(int ch, uint32_t duty);                      /* duty de 0 a 2^bits */
void sdk_ledc_fade(int ch, int up, int steps, int periods_per_step, int step_size);
/* igual, pero parte desde el duty 'start' (el hardware suma o resta step_size cada periods_per_step periodos) */
void sdk_ledc_fade_from(int ch, uint32_t start, int up, int steps, int periods_per_step, int step_size);
uint32_t sdk_ledc_duty_now(int ch);

/* ---- SPI2 (maestro) ---- */
void spi_init(int half_period_cycles);              /* SCK = GPIO6, MOSI = GPIO7, CS0 = GPIO10, MISO = GPIO2 */
void spi_transfer(const uint8_t *tx, uint8_t *rx, int nbytes);

/* ---- I2C (maestro) ---- */
void i2c_init(int sda, int scl);
/* devuelven 0 si el dispositivo respondio (ACK), -1 si no */
int i2c_write(int addr, const uint8_t *data, int n);
int i2c_read(int addr, uint8_t *data, int n);
int i2c_write_read(int addr, const uint8_t *wdata, int wn, uint8_t *rdata, int rn);

/* ---- interrupciones ---- */
typedef void (*irq_handler_t)(void *arg);
/* conecta la fuente 'source' de la matriz a la linea 'cpu_line' (1..31) con prioridad 'prio' (1..15) */
void intr_attach(int source, int cpu_line, int prio, irq_handler_t h, void *arg);
void intr_global_enable(void);
void intr_global_disable(void);

/* ---- chip ---- */
uint32_t reset_reason(void);
const char *reset_reason_name(uint32_t r);
void read_mac(uint8_t mac[6]);
void watchdogs_disable(void);        /* necesario en un chip real con "direct boot" */
void system_reset(void);
uint32_t random32(void);
size_t heap_free_bytes(void);

#ifdef __cplusplus
}
#endif
#endif
