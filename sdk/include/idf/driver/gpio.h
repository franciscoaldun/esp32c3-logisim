/* driver/gpio.h - API de GPIO de ESP-IDF (v5) sobre los registros del modelo */
#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "sdk.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    GPIO_NUM_NC = -1, GPIO_NUM_0 = 0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6,
    GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14,
    GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_20, GPIO_NUM_21, GPIO_NUM_MAX
} gpio_num_t;
typedef enum {
    GPIO_MODE_DISABLE = 0, GPIO_MODE_INPUT = 1, GPIO_MODE_OUTPUT = 2, GPIO_MODE_INPUT_OUTPUT = 3,
    GPIO_MODE_OUTPUT_OD = 6, GPIO_MODE_INPUT_OUTPUT_OD = 7
} gpio_mode_t;
typedef enum { GPIO_PULLUP_DISABLE = 0, GPIO_PULLUP_ENABLE = 1 } gpio_pullup_t;
typedef enum { GPIO_PULLDOWN_DISABLE = 0, GPIO_PULLDOWN_ENABLE = 1 } gpio_pulldown_t;
typedef enum { GPIO_PULLUP_ONLY, GPIO_PULLDOWN_ONLY, GPIO_PULLUP_PULLDOWN, GPIO_FLOATING } gpio_pull_mode_t;
typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
} gpio_config_t;
typedef void (*gpio_isr_t)(void *arg);
#define GPIO_SEL_0 (1ULL << 0)
#define GPIO_SEL_1 (1ULL << 1)
#define GPIO_SEL_2 (1ULL << 2)
#define GPIO_SEL_3 (1ULL << 3)
#define GPIO_SEL_8 (1ULL << 8)
#define GPIO_SEL_9 (1ULL << 9)
esp_err_t gpio_config(const gpio_config_t *cfg);
esp_err_t gpio_reset_pin(gpio_num_t pin);
esp_err_t gpio_set_direction(gpio_num_t pin, gpio_mode_t mode);
esp_err_t gpio_set_level(gpio_num_t pin, uint32_t level);
int gpio_get_level(gpio_num_t pin);
esp_err_t gpio_set_pull_mode(gpio_num_t pin, gpio_pull_mode_t pull);
esp_err_t gpio_pullup_en(gpio_num_t pin);
esp_err_t gpio_pullup_dis(gpio_num_t pin);
esp_err_t gpio_pulldown_en(gpio_num_t pin);
esp_err_t gpio_pulldown_dis(gpio_num_t pin);
esp_err_t gpio_set_intr_type(gpio_num_t pin, gpio_int_type_t type);
esp_err_t gpio_intr_enable(gpio_num_t pin);
esp_err_t gpio_intr_disable(gpio_num_t pin);
esp_err_t gpio_install_isr_service(int intr_alloc_flags);
void gpio_uninstall_isr_service(void);
esp_err_t gpio_isr_handler_add(gpio_num_t pin, gpio_isr_t handler, void *args);
esp_err_t gpio_isr_handler_remove(gpio_num_t pin);
#ifdef __cplusplus
}
#endif
#endif
