/* I2C de ESP-IDF (driver "legacy"): escribe y lee la memoria 24C02 de la placa (0x50, SDA = GPIO4, SCL = GPIO5). */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "i2c";
#define MEMORIA 0x50

void app_main(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 4,
        .scl_io_num = 5,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C listo");

    uint8_t escribir[] = {0x30, 'E', 'S', 'P', '-', 'I', 'D', 'F', 0};
    ESP_ERROR_CHECK(i2c_master_write_to_device(I2C_NUM_0, MEMORIA, escribir, sizeof escribir, pdMS_TO_TICKS(100)));
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t pos = 0x30, leido[9] = {0};
    ESP_ERROR_CHECK(i2c_master_write_read_device(I2C_NUM_0, MEMORIA, &pos, 1, leido, 8, pdMS_TO_TICKS(100)));
    ESP_LOGI(TAG, "lei de la memoria: \"%s\"", (char *)leido);
    esp_err_t r = i2c_master_write_to_device(I2C_NUM_0, 0x23, escribir, 1, pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "direccion 0x23 (no hay nada ahi): %s", esp_err_to_name(r));
}
