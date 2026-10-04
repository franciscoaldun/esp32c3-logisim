/*
 * idf.c - capa "estilo ESP-IDF" para el modelo Logisim: app_main(), un FreeRTOS cooperativo minimo
 * (tareas, vTaskDelay, colas, semaforos) y los drivers gpio / ledc / uart / i2c / log / timer.
 */
#include <string.h>
#include <stdlib.h>
#include "sdk.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_mac.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/i2c.h"

/* ================================================================== errores, log, tiempo, sistema */
const char *esp_err_to_name(esp_err_t c)
{
    switch (c) {
    case ESP_OK: return "ESP_OK";
    case ESP_FAIL: return "ESP_FAIL";
    case ESP_ERR_NO_MEM: return "ESP_ERR_NO_MEM";
    case ESP_ERR_INVALID_ARG: return "ESP_ERR_INVALID_ARG";
    case ESP_ERR_INVALID_STATE: return "ESP_ERR_INVALID_STATE";
    case ESP_ERR_NOT_SUPPORTED: return "ESP_ERR_NOT_SUPPORTED";
    case ESP_ERR_TIMEOUT: return "ESP_ERR_TIMEOUT";
    default: return "ERROR";
    }
}

esp_log_level_t esp_log_level_global = (esp_log_level_t)CONFIG_LOG_DEFAULT_LEVEL;
uint32_t esp_log_timestamp(void) { return sdk_millis(); }
void esp_log_level_set(const char *tag, esp_log_level_t level) { (void)tag; esp_log_level_global = level; }

int64_t esp_timer_get_time(void) { return (int64_t)sdk_micros(); }
void esp_rom_delay_us(uint32_t us) { sdk_delay_us(us); }

void esp_restart(void) { system_reset(); for (;;) { } }

esp_reset_reason_t esp_reset_reason(void)
{
    switch (reset_reason()) {
    case 0x01: return ESP_RST_POWERON;
    case 0x03: case 0x0C: return ESP_RST_SW;
    case 0x07: case 0x08: case 0x0B: case 0x11: return ESP_RST_TASK_WDT;
    case 0x09: case 0x0D: case 0x10: return ESP_RST_WDT;
    default: return ESP_RST_UNKNOWN;
    }
}

static uint32_t heap_min = 0xFFFFFFFF;
uint32_t esp_get_free_heap_size(void)
{
    uint32_t f = (uint32_t)heap_free_bytes();
    if (f < heap_min) heap_min = f;
    return f;
}
uint32_t esp_get_minimum_free_heap_size(void) { esp_get_free_heap_size(); return heap_min; }
uint32_t esp_random(void) { return random32(); }

void esp_chip_info(esp_chip_info_t *o)
{
    o->model = CHIP_ESP32C3;
    o->features = 0;            /* el modelo no tiene radio (WiFi/BLE) */
    o->revision = 4;
    o->cores = 1;
}

esp_err_t esp_flash_get_size(esp_flash_t *chip, uint32_t *out)
{
    (void)chip;
    *out = 1024 * 1024;         /* la FLASH del modelo: 1 MB */
    return ESP_OK;
}

esp_err_t esp_read_mac(uint8_t *mac, esp_mac_type_t type) { (void)type; read_mac(mac); return ESP_OK; }
esp_err_t esp_efuse_mac_get_default(uint8_t *mac) { read_mac(mac); return ESP_OK; }

/* ================================================================== FreeRTOS cooperativo
 * Cada tarea tiene su pila. El cambio de tarea (rtos_switch, en idf_switch.S) guarda ra y s0..s11 en la
 * pila de la tarea que se va y recupera los de la que entra: es exactamente lo que hace un RTOS de verdad,
 * solo que aqui ocurre en vTaskDelay / taskYIELD / al esperar una cola, nunca por interrupcion. */
#define MAX_TASKS 10
enum { T_FREE = 0, T_READY, T_DELAYED, T_SUSPENDED, T_DELETED };
struct rtos_tcb {
    uint32_t sp;
    int state;
    TickType_t wake;
    const char *name;
    void *stack;
    uint32_t stack_size;
    TaskFunction_t fn;
    void *param;
};
static struct rtos_tcb tasks[MAX_TASKS];
static int cur;
static int ntasks = 1;

void rtos_switch(uint32_t *save_sp, uint32_t new_sp);
void rtos_task_start(void);                     /* en idf_switch.S: llama fn(param) y luego rtos_task_exit */

/* secciones criticas: apagan las interrupciones y, al salir, dejan MIE como estaba (sirve tambien dentro
 * de una rutina de interrupcion, donde MIE ya esta en 0) */
static int crit_depth;
static uint32_t crit_saved;
void rtos_enter_critical(void)
{
    uint32_t m;
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(m));
    if (crit_depth++ == 0) crit_saved = m & 8;
}
void rtos_exit_critical(void)
{
    if (crit_depth > 0 && --crit_depth == 0 && crit_saved) __asm__ volatile("csrsi mstatus, 8");
}

TickType_t xTaskGetTickCount(void) { return (TickType_t)(sdk_millis() / portTICK_PERIOD_MS); }
TickType_t xTaskGetTickCountFromISR(void) { return xTaskGetTickCount(); }

static void schedule(void)
{
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        for (int k = 1; k <= MAX_TASKS; k++) {
            int i = (cur + k) % MAX_TASKS;
            struct rtos_tcb *t = &tasks[i];
            if (t->state == T_DELAYED && (int32_t)(now - t->wake) >= 0) t->state = T_READY;
            if (t->state == T_READY) {
                if (i != cur) {
                    int prev = cur;
                    cur = i;
                    rtos_switch(&tasks[prev].sp, tasks[i].sp);
                }
                return;
            }
        }
        /* nadie listo: la CPU espera (en un chip real aqui correria la tarea "idle") */
    }
}

void taskYIELD(void) { schedule(); }

void vTaskDelay(TickType_t ticks)
{
    if (ticks == 0) { schedule(); return; }
    tasks[cur].wake = xTaskGetTickCount() + ticks;
    tasks[cur].state = T_DELAYED;
    schedule();
}

void vTaskDelayUntil(TickType_t *prev, TickType_t inc)
{
    *prev += inc;
    tasks[cur].wake = *prev;
    tasks[cur].state = T_DELAYED;
    schedule();
}
BaseType_t xTaskDelayUntil(TickType_t *prev, TickType_t inc) { vTaskDelayUntil(prev, inc); return pdTRUE; }

void rtos_task_exit(void) { vTaskDelete(0); }

BaseType_t xTaskCreate(TaskFunction_t fn, const char *name, uint32_t stack_bytes, void *param,
                       UBaseType_t prio, TaskHandle_t *handle)
{
    (void)prio;
    int i;
    for (i = 1; i < MAX_TASKS && tasks[i].state != T_FREE && tasks[i].state != T_DELETED; i++) { }
    if (i == MAX_TASKS) return pdFAIL;
    if (stack_bytes < 1024) stack_bytes = 1024;
    if (tasks[i].stack && tasks[i].stack_size < stack_bytes) { free(tasks[i].stack); tasks[i].stack = 0; }
    if (!tasks[i].stack) {
        tasks[i].stack = malloc(stack_bytes);
        tasks[i].stack_size = stack_bytes;
    }
    if (!tasks[i].stack) return pdFAIL;
    uint32_t top = ((uint32_t)tasks[i].stack + tasks[i].stack_size) & ~15u;
    uint32_t *sp = (uint32_t *)(top - 64);
    memset(sp, 0, 64);
    sp[0] = (uint32_t)rtos_task_start;          /* ra: primera "vuelta" de rtos_switch */
    sp[1] = (uint32_t)fn;                       /* s0 */
    sp[2] = (uint32_t)param;                    /* s1 */
    tasks[i].sp = (uint32_t)sp;
    tasks[i].name = name;
    tasks[i].fn = fn;
    tasks[i].param = param;
    tasks[i].state = T_READY;
    ntasks++;
    if (handle) *handle = &tasks[i];
    return pdPASS;
}

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char *name, uint32_t stack_bytes, void *param,
                                   UBaseType_t prio, TaskHandle_t *handle, BaseType_t core)
{
    (void)core;
    return xTaskCreate(fn, name, stack_bytes, param, prio, handle);
}

void vTaskDelete(TaskHandle_t t)
{
    struct rtos_tcb *tcb = t ? t : &tasks[cur];
    if (tcb->state != T_DELETED && tcb->state != T_FREE) ntasks--;
    tcb->state = T_DELETED;
    if (tcb == &tasks[cur]) {
        schedule();
        for (;;) { }                            /* no se vuelve nunca aqui */
    }
}

void vTaskSuspend(TaskHandle_t t) { struct rtos_tcb *x = t ? t : &tasks[cur]; x->state = T_SUSPENDED; if (x == &tasks[cur]) schedule(); }
void vTaskResume(TaskHandle_t t) { if (t && t->state == T_SUSPENDED) t->state = T_READY; }
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return &tasks[cur]; }
const char *pcTaskGetName(TaskHandle_t t) { return (t ? t : &tasks[cur])->name; }
UBaseType_t uxTaskGetNumberOfTasks(void) { return ntasks; }
UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t t) { return (t ? t : &tasks[cur])->stack_size / 2; }
void vTaskStartScheduler(void) { for (;;) vTaskDelay(1000); }

/* ---------------- colas (un buffer circular; quien espera cede la CPU hasta que hay espacio/datos) */
struct rtos_queue { uint8_t *buf; UBaseType_t len, size, head, count; };

QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t item_size)
{
    struct rtos_queue *q = calloc(1, sizeof *q);
    if (!q) return 0;
    q->len = length;
    q->size = item_size;
    if (item_size) {
        q->buf = malloc(length * item_size);
        if (!q->buf) { free(q); return 0; }
    }
    return q;
}

void vQueueDelete(QueueHandle_t q) { if (q) { free(q->buf); free(q); } }

static int q_put(QueueHandle_t q, const void *item, int front)
{
    int ok = 0;
    rtos_enter_critical();
    if (q->count < q->len) {
        UBaseType_t pos;
        if (front) { q->head = (q->head + q->len - 1) % q->len; pos = q->head; }
        else pos = (q->head + q->count) % q->len;
        if (q->size) memcpy(q->buf + pos * q->size, item, q->size);
        q->count++;
        ok = 1;
    }
    rtos_exit_critical();
    return ok;
}

static int q_get(QueueHandle_t q, void *item, int remove)
{
    int ok = 0;
    rtos_enter_critical();
    if (q->count) {
        if (q->size && item) memcpy(item, q->buf + q->head * q->size, q->size);
        if (remove) { q->head = (q->head + 1) % q->len; q->count--; }
        ok = 1;
    }
    rtos_exit_critical();
    return ok;
}

static BaseType_t q_wait(QueueHandle_t q, void *item, TickType_t wait, int send, int front, int remove)
{
    TickType_t start = xTaskGetTickCount();
    for (;;) {
        if (send ? q_put(q, item, front) : q_get(q, item, remove)) return pdTRUE;
        if (wait != portMAX_DELAY && (TickType_t)(xTaskGetTickCount() - start) >= wait) return pdFALSE;
        schedule();                             /* deja correr a las otras tareas mientras espera */
    }
}

BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t w) { return q_wait(q, (void *)item, w, 1, 0, 0); }
BaseType_t xQueueSendToBack(QueueHandle_t q, const void *item, TickType_t w) { return q_wait(q, (void *)item, w, 1, 0, 0); }
BaseType_t xQueueSendToFront(QueueHandle_t q, const void *item, TickType_t w) { return q_wait(q, (void *)item, w, 1, 1, 0); }
BaseType_t xQueueReceive(QueueHandle_t q, void *item, TickType_t w) { return q_wait(q, item, w, 0, 0, 1); }
BaseType_t xQueuePeek(QueueHandle_t q, void *item, TickType_t w) { return q_wait(q, item, w, 0, 0, 0); }
BaseType_t xQueueOverwrite(QueueHandle_t q, const void *item) { xQueueReset(q); return q_put(q, item, 0); }
BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item, BaseType_t *woken) { if (woken) *woken = pdFALSE; return q_put(q, item, 0); }
BaseType_t xQueueReceiveFromISR(QueueHandle_t q, void *item, BaseType_t *woken) { if (woken) *woken = pdFALSE; return q_get(q, item, 1); }
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t q) { return q->count; }
UBaseType_t uxQueueSpacesAvailable(QueueHandle_t q) { return q->len - q->count; }
BaseType_t xQueueReset(QueueHandle_t q) { rtos_enter_critical(); q->head = q->count = 0; rtos_exit_critical(); return pdPASS; }

/* ---------------- semaforos = colas sin datos */
SemaphoreHandle_t xSemaphoreCreateBinary(void) { return xQueueCreate(1, 0); }
SemaphoreHandle_t xSemaphoreCreateMutex(void) { SemaphoreHandle_t s = xQueueCreate(1, 0); if (s) s->count = 1; return s; }
SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t initial)
{
    SemaphoreHandle_t s = xQueueCreate(max, 0);
    if (s) s->count = initial;
    return s;
}
BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t w) { return q_wait(s, 0, w, 0, 0, 1); }
BaseType_t xSemaphoreGive(SemaphoreHandle_t s) { return q_put(s, 0, 0); }
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t s, BaseType_t *woken) { if (woken) *woken = pdFALSE; return q_put(s, 0, 0); }
BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t s, BaseType_t *woken) { if (woken) *woken = pdFALSE; return q_get(s, 0, 1); }

/* ================================================================== driver/gpio */
static gpio_int_type_t pin_intr[NUM_GPIO];
static gpio_isr_t pin_isr[NUM_GPIO];
static void *pin_isr_arg[NUM_GPIO];

static esp_err_t check_pin(gpio_num_t p) { return (p >= 0 && p < NUM_GPIO) ? ESP_OK : ESP_ERR_INVALID_ARG; }

esp_err_t gpio_set_direction(gpio_num_t p, gpio_mode_t m)
{
    if (check_pin(p)) return ESP_ERR_INVALID_ARG;
    if (m & GPIO_MODE_OUTPUT) {
        gpio_output(p);
        if (m & 4) REG_SET_BIT(GPIO_PIN_REG(p), GPIO_PIN_PAD_DRIVER);           /* drenador abierto */
        else REG_CLR_BIT(GPIO_PIN_REG(p), GPIO_PIN_PAD_DRIVER);
    } else {
        uint32_t pu = REG_READ(IO_MUX_GPIO_REG(p)) & FUN_PU;
        gpio_input(p, pu ? 1 : 0);
    }
    return ESP_OK;
}

esp_err_t gpio_reset_pin(gpio_num_t p)
{
    if (check_pin(p)) return ESP_ERR_INVALID_ARG;
    gpio_input(p, 1);                           /* como en ESP-IDF: entrada con pull-up, sin interrupcion */
    gpio_pin_isr(p, 0, 0, 0);
    return ESP_OK;
}

esp_err_t gpio_set_level(gpio_num_t p, uint32_t level) { if (check_pin(p)) return ESP_ERR_INVALID_ARG; gpio_set(p, level ? 1 : 0); return ESP_OK; }
int gpio_get_level(gpio_num_t p) { return check_pin(p) ? 0 : gpio_get(p); }

esp_err_t gpio_set_pull_mode(gpio_num_t p, gpio_pull_mode_t pull)
{
    if (check_pin(p)) return ESP_ERR_INVALID_ARG;
    REG_CLR_BIT(IO_MUX_GPIO_REG(p), FUN_PU | FUN_PD);
    if (pull == GPIO_PULLUP_ONLY || pull == GPIO_PULLUP_PULLDOWN) REG_SET_BIT(IO_MUX_GPIO_REG(p), FUN_PU);
    if (pull == GPIO_PULLDOWN_ONLY || pull == GPIO_PULLUP_PULLDOWN) REG_SET_BIT(IO_MUX_GPIO_REG(p), FUN_PD);
    return ESP_OK;
}
esp_err_t gpio_pullup_en(gpio_num_t p) { REG_SET_BIT(IO_MUX_GPIO_REG(p), FUN_PU); return ESP_OK; }
esp_err_t gpio_pullup_dis(gpio_num_t p) { REG_CLR_BIT(IO_MUX_GPIO_REG(p), FUN_PU); return ESP_OK; }
esp_err_t gpio_pulldown_en(gpio_num_t p) { REG_SET_BIT(IO_MUX_GPIO_REG(p), FUN_PD); return ESP_OK; }
esp_err_t gpio_pulldown_dis(gpio_num_t p) { REG_CLR_BIT(IO_MUX_GPIO_REG(p), FUN_PD); return ESP_OK; }

static void idf_pin_isr(void *arg)
{
    int p = (int)(intptr_t)arg;
    if (pin_isr[p]) pin_isr[p](pin_isr_arg[p]);
}

esp_err_t gpio_set_intr_type(gpio_num_t p, gpio_int_type_t t)
{
    if (check_pin(p)) return ESP_ERR_INVALID_ARG;
    pin_intr[p] = t;
    if (pin_isr[p]) gpio_pin_isr(p, t, idf_pin_isr, (void *)(intptr_t)p);
    return ESP_OK;
}

esp_err_t gpio_intr_enable(gpio_num_t p) { return gpio_set_intr_type(p, pin_intr[p]); }
esp_err_t gpio_intr_disable(gpio_num_t p) { if (check_pin(p)) return ESP_ERR_INVALID_ARG; gpio_pin_isr(p, 0, 0, 0); return ESP_OK; }
esp_err_t gpio_install_isr_service(int flags) { (void)flags; return ESP_OK; }
void gpio_uninstall_isr_service(void) { }

esp_err_t gpio_isr_handler_add(gpio_num_t p, gpio_isr_t h, void *args)
{
    if (check_pin(p)) return ESP_ERR_INVALID_ARG;
    pin_isr[p] = h;
    pin_isr_arg[p] = args;
    if (pin_intr[p]) gpio_pin_isr(p, pin_intr[p], idf_pin_isr, (void *)(intptr_t)p);
    return ESP_OK;
}

esp_err_t gpio_isr_handler_remove(gpio_num_t p) { pin_isr[p] = 0; gpio_pin_isr(p, 0, 0, 0); return ESP_OK; }

esp_err_t gpio_config(const gpio_config_t *c)
{
    uint32_t mask = (uint32_t)c->pin_bit_mask;          /* los 22 pines caben en 32 bits (evita shifts de 64 bits) */
    for (int p = 0; p < NUM_GPIO; p++, mask >>= 1) {
        if (!(mask & 1)) continue;
        gpio_pull_mode_t pull = c->pull_up_en ? (c->pull_down_en ? GPIO_PULLUP_PULLDOWN : GPIO_PULLUP_ONLY)
                                              : (c->pull_down_en ? GPIO_PULLDOWN_ONLY : GPIO_FLOATING);
        gpio_set_direction((gpio_num_t)p, c->mode);
        gpio_set_pull_mode((gpio_num_t)p, pull);
        gpio_set_intr_type((gpio_num_t)p, c->intr_type);
    }
    return ESP_OK;
}

/* ================================================================== driver/ledc */
#define LEDC_BITS_MODELO 8
static uint8_t timer_bits[4] = {8, 8, 8, 8};
static uint8_t ch_timer[6];
static uint32_t ch_duty[6];
static int fade_target[6], fade_cycles[6];

static uint32_t to_model(int ch, uint32_t duty)
{
    int b = timer_bits[ch_timer[ch]];
    uint32_t d = b > LEDC_BITS_MODELO ? duty >> (b - LEDC_BITS_MODELO) : duty << (LEDC_BITS_MODELO - b);
    return d > 256 ? 256 : d;
}

esp_err_t ledc_timer_config(const ledc_timer_config_t *c)
{
    if (c->timer_num >= LEDC_TIMER_MAX) return ESP_ERR_INVALID_ARG;
    timer_bits[c->timer_num] = c->duty_resolution ? c->duty_resolution : 8;
    sdk_ledc_timer(c->timer_num, LEDC_BITS_MODELO, 1);      /* periodo: 256 ciclos */
    return ESP_OK;
}

esp_err_t ledc_channel_config(const ledc_channel_config_t *c)
{
    if (c->channel >= LEDC_CHANNEL_MAX) return ESP_ERR_INVALID_ARG;
    ch_timer[c->channel] = c->timer_sel;
    ch_duty[c->channel] = c->duty;
    sdk_ledc_channel(c->channel, c->timer_sel, c->gpio_num, to_model(c->channel, c->duty));
    return ESP_OK;
}

esp_err_t ledc_set_duty(ledc_mode_t m, ledc_channel_t ch, uint32_t duty) { (void)m; ch_duty[ch] = duty; return ESP_OK; }
esp_err_t ledc_update_duty(ledc_mode_t m, ledc_channel_t ch) { (void)m; sdk_ledc_duty(ch, to_model(ch, ch_duty[ch])); return ESP_OK; }
esp_err_t ledc_set_duty_and_update(ledc_mode_t m, ledc_channel_t ch, uint32_t duty, uint32_t hpoint)
{
    (void)hpoint;
    ledc_set_duty(m, ch, duty);
    return ledc_update_duty(m, ch);
}

uint32_t ledc_get_duty(ledc_mode_t m, ledc_channel_t ch)
{
    (void)m;
    int b = timer_bits[ch_timer[ch]];
    uint32_t d = sdk_ledc_duty_now(ch);
    return b > LEDC_BITS_MODELO ? d << (b - LEDC_BITS_MODELO) : d >> (LEDC_BITS_MODELO - b);
}

esp_err_t ledc_set_freq(ledc_mode_t m, ledc_timer_t t, uint32_t f) { (void)m; (void)t; (void)f; return ESP_OK; }
uint32_t ledc_get_freq(ledc_mode_t m, ledc_timer_t t) { (void)m; (void)t; return 1000 * SIM_CICLOS_POR_MS / 256; }
esp_err_t ledc_stop(ledc_mode_t m, ledc_channel_t ch, uint32_t idle) { (void)m; sdk_ledc_duty(ch, idle ? 256 : 0); return ESP_OK; }
esp_err_t ledc_fade_func_install(int flags) { (void)flags; return ESP_OK; }
void ledc_fade_func_uninstall(void) { }

esp_err_t ledc_set_fade_with_time(ledc_mode_t m, ledc_channel_t ch, uint32_t target, int ms)
{
    (void)m;
    fade_target[ch] = to_model(ch, target);
    fade_cycles[ch] = ms * SIM_CICLOS_POR_MS;
    ch_duty[ch] = target;
    return ESP_OK;
}

esp_err_t ledc_set_fade_with_step(ledc_mode_t m, ledc_channel_t ch, uint32_t target, uint32_t scale, uint32_t cycle_num)
{
    (void)scale;
    return ledc_set_fade_with_time(m, ch, target, (int)(cycle_num * 256 / SIM_CICLOS_POR_MS));
}

esp_err_t ledc_fade_start(ledc_mode_t m, ledc_channel_t ch, ledc_fade_mode_t mode)
{
    (void)m;
    int now = (int)sdk_ledc_duty_now(ch), tgt = fade_target[ch];
    int diff = tgt > now ? tgt - now : now - tgt;
    if (diff == 0) return ESP_OK;
    /* el hardware del LEDC cambia el duty una vez por periodo del PWM (256 ciclos en el modelo):
     * con tiempo de sobra sube de a 1 cada 'cyc' periodos; si no alcanza, sube de a 'scale' por periodo */
    int periodos = fade_cycles[ch] / 256;
    if (periodos < 1) periodos = 1;
    int steps, scale, cyc;
    if (periodos >= diff) { steps = diff; scale = 1; cyc = periodos / diff; }
    else { steps = periodos; scale = diff / periodos; cyc = 1; }
    if (steps > 1023) steps = 1023;
    if (cyc > 1023) cyc = 1023;
    if (scale > 1023) scale = 1023;
    int resto = diff - steps * scale;                /* lo que no calza en pasos iguales se salta al inicio */
    if (resto < 0) resto = 0;
    int inicio = tgt > now ? now + resto : now - resto;
    sdk_ledc_fade_from(ch, (uint32_t)inicio, tgt > now, steps, cyc, scale);   /* lo hace el hardware */
    if (mode == LEDC_FADE_WAIT_DONE) {
        while ((int)sdk_ledc_duty_now(ch) != tgt) vTaskDelay(1);
    }
    return ESP_OK;
}

/* ================================================================== driver/uart */
esp_err_t uart_driver_install(uart_port_t p, int rx, int tx, int qs, void *q, int f) { (void)p; (void)rx; (void)tx; (void)qs; (void)q; (void)f; return ESP_OK; }
esp_err_t uart_driver_delete(uart_port_t p) { (void)p; return ESP_OK; }
esp_err_t uart_param_config(uart_port_t p, const uart_config_t *c) { (void)p; (void)c; return ESP_OK; }
esp_err_t uart_set_pin(uart_port_t p, int tx, int rx, int rts, int cts) { (void)p; (void)tx; (void)rx; (void)rts; (void)cts; return ESP_OK; }

int uart_write_bytes(uart_port_t p, const void *src, size_t n)
{
    uart_write_n(p, (const char *)src, (int)n, 0);
    return (int)n;
}

int uart_read_bytes(uart_port_t p, void *buf, uint32_t n, TickType_t wait)
{
    uint8_t *b = buf;
    uint32_t got = 0;
    TickType_t start = xTaskGetTickCount();
    while (got < n) {
        int c = uart_getc(p);
        if (c >= 0) { b[got++] = (uint8_t)c; continue; }
        if ((TickType_t)(xTaskGetTickCount() - start) >= wait) break;
        taskYIELD();
    }
    return (int)got;
}

esp_err_t uart_get_buffered_data_len(uart_port_t p, size_t *size) { *size = uart_available(p); return ESP_OK; }
esp_err_t uart_flush(uart_port_t p) { while (uart_getc(p) >= 0) { } return ESP_OK; }
esp_err_t uart_flush_input(uart_port_t p) { return uart_flush(p); }
esp_err_t uart_wait_tx_done(uart_port_t p, TickType_t t) { (void)p; (void)t; return ESP_OK; }

/* ================================================================== driver/i2c (legacy) */
static int i2c_sda = 4, i2c_scl = 5;
esp_err_t i2c_param_config(i2c_port_t p, const i2c_config_t *c) { (void)p; i2c_sda = c->sda_io_num; i2c_scl = c->scl_io_num; return ESP_OK; }
esp_err_t i2c_driver_install(i2c_port_t p, i2c_mode_t m, size_t a, size_t b, int f)
{
    (void)p; (void)a; (void)b; (void)f;
    if (m != I2C_MODE_MASTER) return ESP_ERR_NOT_SUPPORTED;
    i2c_init(i2c_sda, i2c_scl);
    return ESP_OK;
}
esp_err_t i2c_driver_delete(i2c_port_t p) { (void)p; return ESP_OK; }
esp_err_t i2c_master_write_to_device(i2c_port_t p, uint8_t a, const uint8_t *b, size_t n, TickType_t t)
{
    (void)p; (void)t;
    return i2c_write(a, b, (int)n) == 0 ? ESP_OK : ESP_FAIL;
}
esp_err_t i2c_master_read_from_device(i2c_port_t p, uint8_t a, uint8_t *b, size_t n, TickType_t t)
{
    (void)p; (void)t;
    return i2c_read(a, b, (int)n) == 0 ? ESP_OK : ESP_FAIL;
}
esp_err_t i2c_master_write_read_device(i2c_port_t p, uint8_t a, const uint8_t *w, size_t wn, uint8_t *r, size_t rn, TickType_t t)
{
    (void)p; (void)t;
    return i2c_write_read(a, w, (int)wn, r, (int)rn) == 0 ? ESP_OK : ESP_FAIL;
}

/* ================================================================== arranque */
void app_main(void);

int main(void)
{
    tasks[0].state = T_READY;
    tasks[0].name = "main";
    tasks[0].stack_size = 0x4000;
    app_main();
    /* como en ESP-IDF: si app_main termina, su tarea se borra y las demas siguen */
    vTaskDelete(0);
    return 0;
}
