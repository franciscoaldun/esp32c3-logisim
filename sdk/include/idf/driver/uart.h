/* driver/uart.h - API de UART de ESP-IDF (lo basico) sobre la UART0 del modelo (terminal de la placa) */
#ifndef DRIVER_UART_H
#define DRIVER_UART_H
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { UART_NUM_0 = 0, UART_NUM_1 = 1, UART_NUM_MAX } uart_port_t;
typedef enum { UART_DATA_5_BITS, UART_DATA_6_BITS, UART_DATA_7_BITS, UART_DATA_8_BITS } uart_word_length_t;
typedef enum { UART_PARITY_DISABLE = 0, UART_PARITY_EVEN = 2, UART_PARITY_ODD = 3 } uart_parity_t;
typedef enum { UART_STOP_BITS_1 = 1, UART_STOP_BITS_1_5 = 2, UART_STOP_BITS_2 = 3 } uart_stop_bits_t;
typedef enum { UART_HW_FLOWCTRL_DISABLE = 0, UART_HW_FLOWCTRL_RTS, UART_HW_FLOWCTRL_CTS, UART_HW_FLOWCTRL_CTS_RTS } uart_hw_flowcontrol_t;
typedef enum { UART_SCLK_DEFAULT = 0, UART_SCLK_APB = 0, UART_SCLK_XTAL = 1 } uart_sclk_t;
typedef struct {
    int baud_rate;
    uart_word_length_t data_bits;
    uart_parity_t parity;
    uart_stop_bits_t stop_bits;
    uart_hw_flowcontrol_t flow_ctrl;
    uint8_t rx_flow_ctrl_thresh;
    uart_sclk_t source_clk;
} uart_config_t;
#define UART_PIN_NO_CHANGE (-1)
esp_err_t uart_driver_install(uart_port_t port, int rx_buffer_size, int tx_buffer_size, int queue_size, void *queue, int flags);
esp_err_t uart_driver_delete(uart_port_t port);
esp_err_t uart_param_config(uart_port_t port, const uart_config_t *cfg);
esp_err_t uart_set_pin(uart_port_t port, int tx, int rx, int rts, int cts);
int uart_write_bytes(uart_port_t port, const void *src, size_t size);
int uart_read_bytes(uart_port_t port, void *buf, uint32_t length, TickType_t ticks_to_wait);
esp_err_t uart_get_buffered_data_len(uart_port_t port, size_t *size);
esp_err_t uart_flush(uart_port_t port);
esp_err_t uart_flush_input(uart_port_t port);
esp_err_t uart_wait_tx_done(uart_port_t port, TickType_t ticks_to_wait);
#ifdef __cplusplus
}
#endif
#endif
