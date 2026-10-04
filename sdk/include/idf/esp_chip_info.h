#ifndef ESP_CHIP_INFO_H
#define ESP_CHIP_INFO_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { CHIP_ESP32 = 1, CHIP_ESP32S2 = 2, CHIP_ESP32S3 = 9, CHIP_ESP32C3 = 5 } esp_chip_model_t;
#define CHIP_FEATURE_EMB_FLASH  (1UL << 0)
#define CHIP_FEATURE_WIFI_BGN   (1UL << 1)
#define CHIP_FEATURE_BLE        (1UL << 4)
#define CHIP_FEATURE_BT         (1UL << 5)
#define CHIP_FEATURE_IEEE802154 (1UL << 6)
#define CHIP_FEATURE_EMB_PSRAM  (1UL << 7)
typedef struct { esp_chip_model_t model; uint32_t features; uint16_t revision; uint8_t cores; } esp_chip_info_t;
void esp_chip_info(esp_chip_info_t *out);   /* en el modelo: sin radio (features = 0), 1 nucleo */
#ifdef __cplusplus
}
#endif
#endif
