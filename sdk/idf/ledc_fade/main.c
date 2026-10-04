/* LEDC de ESP-IDF: un canal con duty fijo y otro que sube y baja solo (fade hecho por el hardware). */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "ledc";

void app_main(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ledc_channel_config_t fijo = {
        .gpio_num = 0, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0, .duty = 2048, .hpoint = 0,          /* 2048 / 8192 = 25 % */
    };
    ESP_ERROR_CHECK(ledc_channel_config(&fijo));
    ledc_channel_config_t respira = fijo;
    respira.gpio_num = 18;
    respira.channel = LEDC_CHANNEL_1;
    respira.duty = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&respira));
    ledc_fade_func_install(0);
    ESP_LOGI(TAG, "GPIO0 al 25%%, GPIO18 sube y baja");
    for (;;) {
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 8191, 3000);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, LEDC_FADE_NO_WAIT);
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_LOGI(TAG, "arriba: duty = %lu", (unsigned long)ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
        ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0, 3000);
        ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, LEDC_FADE_NO_WAIT);
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_LOGI(TAG, "abajo: duty = %lu", (unsigned long)ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
    }
}
