#ifndef ESP_TIMER_H
#define ESP_TIMER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int64_t esp_timer_get_time(void);       /* microsegundos (simulados) desde el arranque */
#ifdef __cplusplus
}
#endif
#endif
