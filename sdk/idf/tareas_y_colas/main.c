/* Dos tareas de FreeRTOS + una cola alimentada por una interrupcion de GPIO.
 *  - tarea "lenta": parpadea GPIO3 cada 800 ms
 *  - tarea "rapida": parpadea GPIO1 cada 300 ms
 *  - al presionar BOOT (GPIO9) la interrupcion manda el numero de pin a una cola, y app_main lo recibe.
 * En Logisim mira el registro sp (x02_sp en la pestana State): cambia cada vez que el RTOS cambia de tarea. */
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "tareas";
static QueueHandle_t cola_eventos;

static void IRAM_ATTR al_presionar(void *arg)
{
    uint32_t pin = (uint32_t)arg;
    xQueueSendFromISR(cola_eventos, &pin, NULL);
}

static void parpadeo(void *arg)
{
    int pin = (int)arg;
    int periodo = pin == 3 ? 800 : 300;
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    int nivel = 0;
    for (;;) {
        gpio_set_level(pin, nivel);
        nivel = !nivel;
        vTaskDelay(pdMS_TO_TICKS(periodo));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "creando tareas");
    cola_eventos = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(parpadeo, "lenta", 2048, (void *)3, 5, NULL);
    xTaskCreate(parpadeo, "rapida", 2048, (void *)1, 5, NULL);

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << 9,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(9, al_presionar, (void *)9);
    ESP_LOGI(TAG, "%u tareas corriendo; presiona BOOT", (unsigned)uxTaskGetNumberOfTasks());

    uint32_t pin;
    for (;;) {
        if (xQueueReceive(cola_eventos, &pin, portMAX_DELAY)) {
            ESP_LOGI(TAG, "interrupcion en GPIO%" PRIu32 ", nivel %d", pin, gpio_get_level(pin));
        }
    }
}
