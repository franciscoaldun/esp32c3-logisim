/* FreeRTOS.h - un FreeRTOS "de juguete" para el modelo: tareas cooperativas (cambian en vTaskDelay,
 * taskYIELD y al esperar una cola), colas y semaforos. Sin prioridades ni expropiacion. */
#ifndef FREERTOS_H
#define FREERTOS_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_attr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t StackType_t;
#define configTICK_RATE_HZ      CONFIG_FREERTOS_HZ
#define portTICK_PERIOD_MS      ((TickType_t)1000 / configTICK_RATE_HZ)
#define portTICK_RATE_MS        portTICK_PERIOD_MS
#define pdMS_TO_TICKS(ms)       ((TickType_t)(((uint64_t)(ms) * configTICK_RATE_HZ) / 1000))
#define pdTICKS_TO_MS(t)        ((uint32_t)((uint64_t)(t) * 1000 / configTICK_RATE_HZ))
#define portMAX_DELAY           ((TickType_t)0xFFFFFFFF)
#define pdFALSE 0
#define pdTRUE  1
#define pdPASS  1
#define pdFAIL  0
#define errQUEUE_EMPTY 0
#define errQUEUE_FULL  0
#define configMAX_PRIORITIES 25
#define configMINIMAL_STACK_SIZE 768
#define tskIDLE_PRIORITY 0
#define tskNO_AFFINITY 0x7FFFFFFF
typedef struct { int unused; } portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED {0}
void rtos_enter_critical(void);
void rtos_exit_critical(void);
#define portENTER_CRITICAL(m)      rtos_enter_critical()
#define portEXIT_CRITICAL(m)       rtos_exit_critical()
#define portENTER_CRITICAL_ISR(m)  rtos_enter_critical()
#define portEXIT_CRITICAL_ISR(m)   rtos_exit_critical()
#define taskENTER_CRITICAL(m)      rtos_enter_critical()
#define taskEXIT_CRITICAL(m)       rtos_exit_critical()
#define portYIELD_FROM_ISR(x)      ((void)(x))
#define portYIELD_FROM_ISR_ARG(x)  ((void)(x))
#define configASSERT(x) do { if (!(x)) { printf("configASSERT fallo: %s\n", #x); for (;;) { } } } while (0)
#ifdef __cplusplus
}
#endif
#endif
