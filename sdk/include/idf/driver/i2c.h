/* driver/i2c.h - API I2C "legacy" de ESP-IDF (modo maestro) sobre el I2C del modelo */
#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { I2C_NUM_0 = 0, I2C_NUM_MAX } i2c_port_t;
typedef enum { I2C_MODE_SLAVE = 0, I2C_MODE_MASTER, I2C_MODE_MAX } i2c_mode_t;
typedef struct {
    i2c_mode_t mode;
    int sda_io_num;
    int scl_io_num;
    bool sda_pullup_en;
    bool scl_pullup_en;
    union {
        struct { uint32_t clk_speed; } master;
        struct { uint8_t addr_10bit_en; uint16_t slave_addr; uint32_t maximum_speed; } slave;
    };
    uint32_t clk_flags;
} i2c_config_t;
esp_err_t i2c_param_config(i2c_port_t port, const i2c_config_t *cfg);
esp_err_t i2c_driver_install(i2c_port_t port, i2c_mode_t mode, size_t slv_rx_buf_len, size_t slv_tx_buf_len, int intr_alloc_flags);
esp_err_t i2c_driver_delete(i2c_port_t port);
esp_err_t i2c_master_write_to_device(i2c_port_t port, uint8_t addr, const uint8_t *buf, size_t size, TickType_t ticks);
esp_err_t i2c_master_read_from_device(i2c_port_t port, uint8_t addr, uint8_t *buf, size_t size, TickType_t ticks);
esp_err_t i2c_master_write_read_device(i2c_port_t port, uint8_t addr, const uint8_t *wbuf, size_t wsize,
                                       uint8_t *rbuf, size_t rsize, TickType_t ticks);
#ifdef __cplusplus
}
#endif
#endif
