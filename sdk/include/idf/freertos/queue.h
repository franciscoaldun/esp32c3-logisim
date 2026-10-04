#ifndef FREERTOS_QUEUE_H
#define FREERTOS_QUEUE_H
#include "FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct rtos_queue *QueueHandle_t;
QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t item_size);
void vQueueDelete(QueueHandle_t q);
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t wait);
BaseType_t xQueueSendToBack(QueueHandle_t q, const void *item, TickType_t wait);
BaseType_t xQueueSendToFront(QueueHandle_t q, const void *item, TickType_t wait);
BaseType_t xQueueOverwrite(QueueHandle_t q, const void *item);
BaseType_t xQueueReceive(QueueHandle_t q, void *item, TickType_t wait);
BaseType_t xQueuePeek(QueueHandle_t q, void *item, TickType_t wait);
BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item, BaseType_t *woken);
BaseType_t xQueueReceiveFromISR(QueueHandle_t q, void *item, BaseType_t *woken);
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t q);
UBaseType_t uxQueueSpacesAvailable(QueueHandle_t q);
BaseType_t xQueueReset(QueueHandle_t q);
#define xQueueSendToBackFromISR xQueueSendFromISR
#ifdef __cplusplus
}
#endif
#endif
