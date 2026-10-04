/* esp_log.h - ESP_LOGI y compania, con el mismo formato que ESP-IDF: "I (tiempo_ms) TAG: mensaje" */
#ifndef ESP_LOG_H
#define ESP_LOG_H
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { ESP_LOG_NONE, ESP_LOG_ERROR, ESP_LOG_WARN, ESP_LOG_INFO, ESP_LOG_DEBUG, ESP_LOG_VERBOSE } esp_log_level_t;
uint32_t esp_log_timestamp(void);
void esp_log_level_set(const char *tag, esp_log_level_t level);
extern esp_log_level_t esp_log_level_global;
#define ESP_LOG_LEVEL_(lvl, letter, tag, fmt, ...) do { if (esp_log_level_global >= (lvl)) \
        printf(letter " (%lu) %s: " fmt "\n", (unsigned long)esp_log_timestamp(), tag, ##__VA_ARGS__); } while (0)
#define ESP_LOGE(tag, fmt, ...) ESP_LOG_LEVEL_(ESP_LOG_ERROR, "E", tag, fmt, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) ESP_LOG_LEVEL_(ESP_LOG_WARN, "W", tag, fmt, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) ESP_LOG_LEVEL_(ESP_LOG_INFO, "I", tag, fmt, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) ESP_LOG_LEVEL_(ESP_LOG_DEBUG, "D", tag, fmt, ##__VA_ARGS__)
#define ESP_LOGV(tag, fmt, ...) ESP_LOG_LEVEL_(ESP_LOG_VERBOSE, "V", tag, fmt, ##__VA_ARGS__)
#define ESP_EARLY_LOGI ESP_LOGI
#define ESP_EARLY_LOGE ESP_LOGE
#define ESP_DRAM_LOGI ESP_LOGI
#ifdef __cplusplus
}
#endif
#endif
