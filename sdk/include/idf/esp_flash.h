#ifndef ESP_FLASH_H
#define ESP_FLASH_H
#include <stdint.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct esp_flash esp_flash_t;
esp_err_t esp_flash_get_size(esp_flash_t *chip, uint32_t *out_size);     /* 1 MB en el modelo */
#ifdef __cplusplus
}
#endif
#endif
