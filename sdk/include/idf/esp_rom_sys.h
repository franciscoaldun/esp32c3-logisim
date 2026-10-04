#ifndef ESP_ROM_SYS_H
#define ESP_ROM_SYS_H
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
void esp_rom_delay_us(uint32_t us);
#define esp_rom_printf printf
#ifdef __cplusplus
}
#endif
#endif
