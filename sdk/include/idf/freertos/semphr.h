#ifndef FREERTOS_SEMPHR_H
#define FREERTOS_SEMPHR_H
#include "queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef QueueHandle_t SemaphoreHandle_t;
SemaphoreHandle_t xSemaphoreCreateBinary(void);
SemaphoreHandle_t xSemaphoreCreateMutex(void);
SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t initial);
BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t wait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t s);
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t s, BaseType_t *woken);
BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t s, BaseType_t *woken);
#define xSemaphoreCreateRecursiveMutex xSemaphoreCreateMutex
#define xSemaphoreTakeRecursive xSemaphoreTake
#define xSemaphoreGiveRecursive xSemaphoreGive
#define vSemaphoreDelete vQueueDelete
#define uxSemaphoreGetCount uxQueueMessagesWaiting
#ifdef __cplusplus
}
#endif
#endif
