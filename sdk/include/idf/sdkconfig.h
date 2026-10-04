/* sdkconfig.h - configuracion "de menuconfig" para el modelo Logisim (valores por defecto razonables) */
#ifndef SDKCONFIG_H
#define SDKCONFIG_H
#define CONFIG_IDF_TARGET "esp32c3"
#define CONFIG_IDF_TARGET_ESP32C3 1
#define CONFIG_FREERTOS_HZ 100
#define CONFIG_LOG_DEFAULT_LEVEL 3
#define CONFIG_ESP_CONSOLE_UART_NUM 0
/* valores de los ejemplos oficiales (blink) */
#ifndef CONFIG_BLINK_GPIO
#define CONFIG_BLINK_GPIO 8
#endif
#ifndef CONFIG_BLINK_PERIOD
#define CONFIG_BLINK_PERIOD 1000
#endif
#if !defined(CONFIG_BLINK_LED_STRIP) && !defined(CONFIG_BLINK_LED_GPIO)
#define CONFIG_BLINK_LED_GPIO 1
#endif
#endif
