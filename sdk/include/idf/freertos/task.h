#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H
#include "FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct rtos_tcb *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);
BaseType_t xTaskCreate(TaskFunction_t fn, const char *name, uint32_t stack_bytes, void *param,
                       UBaseType_t priority, TaskHandle_t *handle);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char *name, uint32_t stack_bytes, void *param,
                                   UBaseType_t priority, TaskHandle_t *handle, BaseType_t core);
void vTaskDelete(TaskHandle_t t);
void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t *previous_wake, TickType_t increment);
BaseType_t xTaskDelayUntil(TickType_t *previous_wake, TickType_t increment);
TickType_t xTaskGetTickCount(void);
TickType_t xTaskGetTickCountFromISR(void);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
const char *pcTaskGetName(TaskHandle_t t);
UBaseType_t uxTaskGetNumberOfTasks(void);
UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t t);
void vTaskSuspend(TaskHandle_t t);
void vTaskResume(TaskHandle_t t);
void taskYIELD(void);
void vTaskStartScheduler(void);
#define pcTaskGetTaskName pcTaskGetName
#ifdef __cplusplus
}
#endif
#endif
