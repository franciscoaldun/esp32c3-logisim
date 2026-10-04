/*
 * esp32c3.h - Direcciones de registros del ESP32-C3 (subconjunto implementado en el modelo Logisim).
 * Los nombres y direcciones son los del Manual de Referencia Tecnica (TRM) y los encabezados de ESP-IDF,
 * asi que el mismo codigo "bare-metal" sirve tambien en un ESP32-C3 real.
 */
#ifndef ESP32C3_H
#define ESP32C3_H
#include <stdint.h>

#define REG(a)              (*(volatile uint32_t *)(a))
#define REG_WRITE(a, v)     (REG(a) = (uint32_t)(v))
#define REG_READ(a)         (REG(a))
#define REG_SET_BIT(a, b)   (REG(a) |= (uint32_t)(b))
#define REG_CLR_BIT(a, b)   (REG(a) &= ~(uint32_t)(b))
#define BIT(n)              (1UL << (n))

/* ---------------- mapa de memoria ---------------- */
#define SOC_IROM_LOW        0x42000000   /* flash, vista de instrucciones */
#define SOC_DROM_LOW        0x3C000000   /* flash, vista de datos        */
#define SOC_IROM_MASK_LOW   0x40000000   /* ROM interna                  */
#define SOC_DRAM_LOW        0x3FC80000   /* SRAM, vista de datos         */
#define SOC_IRAM_LOW        0x40380000   /* SRAM, vista de instrucciones */
#define SOC_RTC_DRAM_LOW    0x50000000   /* RTC FAST memory (8 KB)       */

/* ---------------- bases de perifericos ---------------- */
#define DR_REG_UART_BASE        0x60000000
#define DR_REG_GPIO_BASE        0x60004000
#define DR_REG_RTCCNTL_BASE     0x60008000
#define DR_REG_EFUSE_BASE       0x60008800
#define DR_REG_IO_MUX_BASE      0x60009000
#define DR_REG_UART1_BASE       0x60010000
#define DR_REG_I2C_EXT_BASE     0x60013000
#define DR_REG_LEDC_BASE        0x60019000
#define DR_REG_TIMERGROUP0_BASE 0x6001F000
#define DR_REG_TIMERGROUP1_BASE 0x60020000
#define DR_REG_SYSTIMER_BASE    0x60023000
#define DR_REG_SPI2_BASE        0x60024000
#define DR_REG_SYSCON_BASE      0x60026000
#define DR_REG_SHA_BASE         0x6003B000
#define DR_REG_USB_SERIAL_JTAG_BASE 0x60043000
#define DR_REG_SYSTEM_BASE      0x600C0000
#define DR_REG_INTERRUPT_BASE   0x600C2000

/* ---------------- UART (i = 0, 1) ---------------- */
#define REG_UART_BASE(i)        ((i) == 0 ? DR_REG_UART_BASE : DR_REG_UART1_BASE)
#define UART_FIFO_REG(i)        (REG_UART_BASE(i) + 0x00)
#define UART_INT_RAW_REG(i)     (REG_UART_BASE(i) + 0x04)
#define UART_INT_ST_REG(i)      (REG_UART_BASE(i) + 0x08)
#define UART_INT_ENA_REG(i)     (REG_UART_BASE(i) + 0x0C)
#define UART_INT_CLR_REG(i)     (REG_UART_BASE(i) + 0x10)
#define UART_CLKDIV_REG(i)      (REG_UART_BASE(i) + 0x14)
#define UART_STATUS_REG(i)      (REG_UART_BASE(i) + 0x1C)
#define UART_CONF0_REG(i)       (REG_UART_BASE(i) + 0x20)
#define UART_CONF1_REG(i)       (REG_UART_BASE(i) + 0x24)
#define UART_MEM_CONF_REG(i)    (REG_UART_BASE(i) + 0x60)
#define UART_DATE_REG(i)        (REG_UART_BASE(i) + 0x7C)
#define UART_RXFIFO_CNT(st)     ((st) & 0x3FF)
#define UART_TXFIFO_CNT(st)     (((st) >> 16) & 0x3FF)
#define UART_RXFIFO_FULL_INT    BIT(0)
#define UART_TXFIFO_EMPTY_INT   BIT(1)
#define UART_FRM_ERR_INT        BIT(3)
#define UART_RXFIFO_OVF_INT     BIT(4)
#define UART_RXFIFO_TOUT_INT    BIT(8)
#define UART_TX_DONE_INT        BIT(14)
#define UART_LOOPBACK           BIT(14)
#define UART_RX_TOUT_EN         BIT(21)

/* ---------------- GPIO ---------------- */
#define GPIO_OUT_REG            (DR_REG_GPIO_BASE + 0x04)
#define GPIO_OUT_W1TS_REG       (DR_REG_GPIO_BASE + 0x08)
#define GPIO_OUT_W1TC_REG       (DR_REG_GPIO_BASE + 0x0C)
#define GPIO_ENABLE_REG         (DR_REG_GPIO_BASE + 0x20)
#define GPIO_ENABLE_W1TS_REG    (DR_REG_GPIO_BASE + 0x24)
#define GPIO_ENABLE_W1TC_REG    (DR_REG_GPIO_BASE + 0x28)
#define GPIO_STRAP_REG          (DR_REG_GPIO_BASE + 0x38)
#define GPIO_IN_REG             (DR_REG_GPIO_BASE + 0x3C)
#define GPIO_STATUS_REG         (DR_REG_GPIO_BASE + 0x44)
#define GPIO_STATUS_W1TS_REG    (DR_REG_GPIO_BASE + 0x48)
#define GPIO_STATUS_W1TC_REG    (DR_REG_GPIO_BASE + 0x4C)
#define GPIO_PCPU_INT_REG       (DR_REG_GPIO_BASE + 0x5C)
#define GPIO_PIN_REG(n)         (DR_REG_GPIO_BASE + 0x74 + 4 * (n))
#define GPIO_FUNC_IN_SEL_CFG_REG(s)  (DR_REG_GPIO_BASE + 0x154 + 4 * (s))
#define GPIO_FUNC_OUT_SEL_CFG_REG(n) (DR_REG_GPIO_BASE + 0x554 + 4 * (n))
#define GPIO_PIN_PAD_DRIVER     BIT(2)
#define GPIO_PIN_INT_TYPE_S     7
#define GPIO_PIN_INT_ENA_S      13
/* tipos de interrupcion de un pin (mismos nombres y valores que gpio_int_type_t de ESP-IDF) */
#ifndef GPIO_INT_TYPE_DEFINED
#define GPIO_INT_TYPE_DEFINED
typedef enum {
    GPIO_INTR_DISABLE = 0, GPIO_INTR_POSEDGE = 1, GPIO_INTR_NEGEDGE = 2, GPIO_INTR_ANYEDGE = 3,
    GPIO_INTR_LOW_LEVEL = 4, GPIO_INTR_HIGH_LEVEL = 5, GPIO_INTR_MAX
} gpio_int_type_t;
#endif
#define NUM_GPIO                22
#define SIG_GPIO_OUT_IDX        128
#define GPIO_FUNC_IN_SEL_S      0
#define GPIO_SIG_IN_SEL         BIT(6)
#define U0RXD_IN_IDX            6
#define U0TXD_OUT_IDX           6
#define U1RXD_IN_IDX            9
#define U1TXD_OUT_IDX           9
#define LEDC_LS_SIG_OUT0_IDX    45
#define I2CEXT0_SCL_OUT_IDX     53
#define I2CEXT0_SDA_OUT_IDX     54
#define FSPICLK_OUT_IDX         63
#define FSPIQ_IN_IDX            64
#define FSPID_OUT_IDX           65
#define FSPICS0_OUT_IDX         68

/* ---------------- IO MUX ---------------- */
#define IO_MUX_GPIO_REG(n)      (DR_REG_IO_MUX_BASE + 0x04 + 4 * (n))
#define MCU_SEL_S               12
#define MCU_SEL_M               (7 << 12)
#define FUN_IE                  BIT(9)
#define FUN_PU                  BIT(8)
#define FUN_PD                  BIT(7)
#define PIN_FUNC_GPIO           1
#define PIN_FUNC_FSPI           2       /* funcion 2 de GPIO2/4/5/6/7/10: FSPIQ/HD/WP/CLK/D/CS0 */

/* ---------------- RTC_CNTL ---------------- */
#define RTC_CNTL_OPTIONS0_REG       (DR_REG_RTCCNTL_BASE + 0x00)
#define RTC_CNTL_SW_SYS_RST         BIT(31)
#define RTC_CNTL_SW_PROCPU_RST      BIT(5)
#define RTC_CNTL_TIME_UPDATE_REG    (DR_REG_RTCCNTL_BASE + 0x0C)
#define RTC_CNTL_TIME_LOW0_REG      (DR_REG_RTCCNTL_BASE + 0x10)
#define RTC_CNTL_TIME_HIGH0_REG     (DR_REG_RTCCNTL_BASE + 0x14)
#define RTC_CNTL_RESET_STATE_REG    (DR_REG_RTCCNTL_BASE + 0x38)
#define RTC_CNTL_INT_ENA_REG        (DR_REG_RTCCNTL_BASE + 0x40)
#define RTC_CNTL_INT_RAW_REG        (DR_REG_RTCCNTL_BASE + 0x44)
#define RTC_CNTL_INT_ST_REG         (DR_REG_RTCCNTL_BASE + 0x48)
#define RTC_CNTL_INT_CLR_REG        (DR_REG_RTCCNTL_BASE + 0x4C)
#define RTC_CNTL_STORE0_REG         (DR_REG_RTCCNTL_BASE + 0x50)
#define RTC_CNTL_WDTCONFIG0_REG     (DR_REG_RTCCNTL_BASE + 0x90)
#define RTC_CNTL_WDTCONFIG1_REG     (DR_REG_RTCCNTL_BASE + 0x94)
#define RTC_CNTL_WDTFEED_REG        (DR_REG_RTCCNTL_BASE + 0xA4)
#define RTC_CNTL_WDTWPROTECT_REG    (DR_REG_RTCCNTL_BASE + 0xA8)
#define RTC_CNTL_SWD_CONF_REG       (DR_REG_RTCCNTL_BASE + 0xAC)
#define RTC_CNTL_SWD_WPROTECT_REG   (DR_REG_RTCCNTL_BASE + 0xB0)
#define RTC_CNTL_WDT_WKEY_VALUE     0x50D83AA1
#define RTC_CNTL_SWD_WKEY_VALUE     0x8F1D312A

/* ---------------- eFuse ---------------- */
#define EFUSE_RD_MAC_SPI_SYS_0_REG  (DR_REG_EFUSE_BASE + 0x44)
#define EFUSE_RD_MAC_SPI_SYS_1_REG  (DR_REG_EFUSE_BASE + 0x48)

/* ---------------- Timer groups (i = 0, 1) ---------------- */
#define REG_TIMG_BASE(i)            ((i) == 0 ? DR_REG_TIMERGROUP0_BASE : DR_REG_TIMERGROUP1_BASE)
#define TIMG_T0CONFIG_REG(i)        (REG_TIMG_BASE(i) + 0x00)
#define TIMG_T0LO_REG(i)            (REG_TIMG_BASE(i) + 0x04)
#define TIMG_T0HI_REG(i)            (REG_TIMG_BASE(i) + 0x08)
#define TIMG_T0UPDATE_REG(i)        (REG_TIMG_BASE(i) + 0x0C)
#define TIMG_T0ALARMLO_REG(i)       (REG_TIMG_BASE(i) + 0x10)
#define TIMG_T0ALARMHI_REG(i)       (REG_TIMG_BASE(i) + 0x14)
#define TIMG_T0LOADLO_REG(i)        (REG_TIMG_BASE(i) + 0x18)
#define TIMG_T0LOADHI_REG(i)        (REG_TIMG_BASE(i) + 0x1C)
#define TIMG_T0LOAD_REG(i)          (REG_TIMG_BASE(i) + 0x20)
#define TIMG_WDTCONFIG0_REG(i)      (REG_TIMG_BASE(i) + 0x48)
#define TIMG_WDTCONFIG1_REG(i)      (REG_TIMG_BASE(i) + 0x4C)
#define TIMG_WDTCONFIG2_REG(i)      (REG_TIMG_BASE(i) + 0x50)
#define TIMG_WDTCONFIG3_REG(i)      (REG_TIMG_BASE(i) + 0x54)
#define TIMG_WDTFEED_REG(i)         (REG_TIMG_BASE(i) + 0x60)
#define TIMG_WDTWPROTECT_REG(i)     (REG_TIMG_BASE(i) + 0x64)
#define TIMG_INT_ENA_TIMERS_REG(i)  (REG_TIMG_BASE(i) + 0x70)
#define TIMG_INT_RAW_TIMERS_REG(i)  (REG_TIMG_BASE(i) + 0x74)
#define TIMG_INT_ST_TIMERS_REG(i)   (REG_TIMG_BASE(i) + 0x78)
#define TIMG_INT_CLR_TIMERS_REG(i)  (REG_TIMG_BASE(i) + 0x7C)
#define TIMG_T0_EN                  BIT(31)
#define TIMG_T0_INCREASE            BIT(30)
#define TIMG_T0_AUTORELOAD          BIT(29)
#define TIMG_T0_DIVIDER_S           13
#define TIMG_T0_ALARM_EN            BIT(10)
#define TIMG_WDT_EN                 BIT(31)
#define TIMG_WDT_STG0_S             29
#define TIMG_WDT_WKEY_VALUE         0x50D83AA1

/* ---------------- SYSTIMER ---------------- */
#define SYSTIMER_CONF_REG           (DR_REG_SYSTIMER_BASE + 0x00)
#define SYSTIMER_UNIT0_OP_REG       (DR_REG_SYSTIMER_BASE + 0x04)
#define SYSTIMER_UNIT0_LOAD_HI_REG  (DR_REG_SYSTIMER_BASE + 0x0C)
#define SYSTIMER_UNIT0_LOAD_LO_REG  (DR_REG_SYSTIMER_BASE + 0x10)
#define SYSTIMER_TARGET0_HI_REG     (DR_REG_SYSTIMER_BASE + 0x1C)
#define SYSTIMER_TARGET0_LO_REG     (DR_REG_SYSTIMER_BASE + 0x20)
#define SYSTIMER_TARGETn_HI_REG(n)  (DR_REG_SYSTIMER_BASE + 0x1C + 8 * (n))
#define SYSTIMER_TARGETn_LO_REG(n)  (DR_REG_SYSTIMER_BASE + 0x20 + 8 * (n))
#define SYSTIMER_TARGETn_CONF_REG(n) (DR_REG_SYSTIMER_BASE + 0x34 + 4 * (n))
#define SYSTIMER_UNIT0_VALUE_HI_REG (DR_REG_SYSTIMER_BASE + 0x40)
#define SYSTIMER_UNIT0_VALUE_LO_REG (DR_REG_SYSTIMER_BASE + 0x44)
#define SYSTIMER_COMPn_LOAD_REG(n)  (DR_REG_SYSTIMER_BASE + 0x50 + 4 * (n))
#define SYSTIMER_UNIT0_LOAD_REG     (DR_REG_SYSTIMER_BASE + 0x5C)
#define SYSTIMER_INT_ENA_REG        (DR_REG_SYSTIMER_BASE + 0x64)
#define SYSTIMER_INT_RAW_REG        (DR_REG_SYSTIMER_BASE + 0x68)
#define SYSTIMER_INT_CLR_REG        (DR_REG_SYSTIMER_BASE + 0x6C)
#define SYSTIMER_INT_ST_REG         (DR_REG_SYSTIMER_BASE + 0x70)
#define SYSTIMER_TIMER_UNIT0_UPDATE BIT(30)
#define SYSTIMER_TIMER_UNIT0_VALUE_VALID BIT(29)
#define SYSTIMER_TARGETn_WORK_EN(n) BIT(24 - (n))
#define SYSTIMER_TARGET_PERIOD_MODE BIT(30)

/* ---------------- LEDC (PWM) ---------------- */
#define LEDC_CHn_CONF0_REG(n)       (DR_REG_LEDC_BASE + 0x14 * (n) + 0x00)
#define LEDC_CHn_HPOINT_REG(n)      (DR_REG_LEDC_BASE + 0x14 * (n) + 0x04)
#define LEDC_CHn_DUTY_REG(n)        (DR_REG_LEDC_BASE + 0x14 * (n) + 0x08)
#define LEDC_CHn_CONF1_REG(n)       (DR_REG_LEDC_BASE + 0x14 * (n) + 0x0C)
#define LEDC_CHn_DUTY_R_REG(n)      (DR_REG_LEDC_BASE + 0x14 * (n) + 0x10)
#define LEDC_TIMERx_CONF_REG(x)     (DR_REG_LEDC_BASE + 0xA0 + 8 * (x))
#define LEDC_TIMERx_VALUE_REG(x)    (DR_REG_LEDC_BASE + 0xA4 + 8 * (x))
#define LEDC_INT_RAW_REG            (DR_REG_LEDC_BASE + 0xC0)
#define LEDC_INT_ST_REG             (DR_REG_LEDC_BASE + 0xC4)
#define LEDC_INT_ENA_REG            (DR_REG_LEDC_BASE + 0xC8)
#define LEDC_INT_CLR_REG            (DR_REG_LEDC_BASE + 0xCC)
#define LEDC_CONF_REG               (DR_REG_LEDC_BASE + 0xD0)
#define LEDC_TIMER_SEL_S            0
#define LEDC_SIG_OUT_EN             BIT(2)
#define LEDC_IDLE_LV                BIT(3)
#define LEDC_PARA_UP                BIT(4)
#define LEDC_DUTY_START             BIT(31)
#define LEDC_DUTY_INC               BIT(30)
#define LEDC_DUTY_NUM_S             20
#define LEDC_DUTY_CYCLE_S           10
#define LEDC_DUTY_SCALE_S           0
#define LEDC_TIMER_DUTY_RES_S       0
#define LEDC_TIMER_CLK_DIV_S        4           /* 10 bits enteros + 8 de fraccion: divisor << 8 */
#define LEDC_TIMER_PAUSE            BIT(22)
#define LEDC_TIMER_RST              BIT(23)
#define LEDC_TIMER_PARA_UP          BIT(25)
#define LEDC_APB_CLK_SEL_S          0
#define LEDC_CLK_EN                 BIT(31)
#define LEDC_DUTY_CHNG_END_INT(n)   BIT(4 + (n))

/* ---------------- SPI2 (maestro) ---------------- */
#define SPI_CMD_REG                 (DR_REG_SPI2_BASE + 0x00)
#define SPI_ADDR_REG                (DR_REG_SPI2_BASE + 0x04)
#define SPI_CTRL_REG                (DR_REG_SPI2_BASE + 0x08)
#define SPI_CLOCK_REG               (DR_REG_SPI2_BASE + 0x0C)
#define SPI_USER_REG                (DR_REG_SPI2_BASE + 0x10)
#define SPI_USER1_REG               (DR_REG_SPI2_BASE + 0x14)
#define SPI_USER2_REG               (DR_REG_SPI2_BASE + 0x18)
#define SPI_MS_DLEN_REG             (DR_REG_SPI2_BASE + 0x1C)
#define SPI_MISC_REG                (DR_REG_SPI2_BASE + 0x20)
#define SPI_DMA_INT_ENA_REG         (DR_REG_SPI2_BASE + 0x34)
#define SPI_DMA_INT_CLR_REG         (DR_REG_SPI2_BASE + 0x38)
#define SPI_DMA_INT_RAW_REG         (DR_REG_SPI2_BASE + 0x3C)
#define SPI_W_REG(i)                (DR_REG_SPI2_BASE + 0x98 + 4 * (i))
#define SPI_USR                     BIT(24)
#define SPI_UPDATE                  BIT(23)
#define SPI_CLK_EQU_SYSCLK          BIT(31)
#define SPI_CLKDIV_PRE_S            18
#define SPI_CLKCNT_N_S              12
#define SPI_CLKCNT_H_S              6
#define SPI_CLKCNT_L_S              0
#define SPI_USR_COMMAND             BIT(31)
#define SPI_USR_ADDR                BIT(30)
#define SPI_USR_DUMMY               BIT(29)
#define SPI_USR_MISO                BIT(28)
#define SPI_USR_MOSI                BIT(27)
#define SPI_DOUTDIN                 BIT(0)
#define SPI_TRANS_DONE_INT          BIT(12)

/* ---------------- I2C (maestro) ---------------- */
#define I2C_SCL_LOW_PERIOD_REG      (DR_REG_I2C_EXT_BASE + 0x00)
#define I2C_CTR_REG                 (DR_REG_I2C_EXT_BASE + 0x04)
#define I2C_SR_REG                  (DR_REG_I2C_EXT_BASE + 0x08)
#define I2C_FIFO_CONF_REG           (DR_REG_I2C_EXT_BASE + 0x18)
#define I2C_DATA_REG                (DR_REG_I2C_EXT_BASE + 0x1C)
#define I2C_INT_RAW_REG             (DR_REG_I2C_EXT_BASE + 0x20)
#define I2C_INT_CLR_REG             (DR_REG_I2C_EXT_BASE + 0x24)
#define I2C_INT_ENA_REG             (DR_REG_I2C_EXT_BASE + 0x28)
#define I2C_INT_STATUS_REG          (DR_REG_I2C_EXT_BASE + 0x2C)
#define I2C_SCL_HIGH_PERIOD_REG     (DR_REG_I2C_EXT_BASE + 0x38)
#define I2C_COMD_REG(n)             (DR_REG_I2C_EXT_BASE + 0x58 + 4 * (n))
#define I2C_MS_MODE                 BIT(4)
#define I2C_TRANS_START             BIT(5)
#define I2C_CONF_UPGATE             BIT(11)
#define I2C_TX_FIFO_RST             BIT(13)
#define I2C_RX_FIFO_RST             BIT(12)
#define I2C_COMMAND_DONE            BIT(31)
#define I2C_END_DETECT_INT          BIT(3)
#define I2C_BYTE_TRANS_DONE_INT     BIT(4)
#define I2C_TRANS_COMPLETE_INT      BIT(7)
#define I2C_NACK_INT                BIT(10)
/* comandos (campo op_code de COMDn, valores del ESP32-C3) */
#define I2C_CMD_RSTART              6
#define I2C_CMD_WRITE               1
#define I2C_CMD_READ                3
#define I2C_CMD_STOP                2
#define I2C_CMD_END                 4
#define I2C_COMMAND(op, ack_val, ack_exp, ack_chk, n) \
    (((op) << 11) | ((ack_val) << 10) | ((ack_exp) << 9) | ((ack_chk) << 8) | (n))
#define I2CEXT0_SCL_IDX             53      /* senal de la matriz GPIO (entrada y salida) */
#define I2CEXT0_SDA_IDX             54

/* ---------------- SYSCON / RNG ---------------- */
#define WDEV_RND_REG                (DR_REG_SYSCON_BASE + 0xB0)

/* ---------------- SYSTEM ---------------- */
#define SYSTEM_PERIP_CLK_EN0_REG    (DR_REG_SYSTEM_BASE + 0x10)
#define SYSTEM_PERIP_RST_EN0_REG    (DR_REG_SYSTEM_BASE + 0x18)
/* bits de SYSTEM_PERIP_CLK_EN0 / SYSTEM_PERIP_RST_EN0 */
#define SYSTEM_UART_CLK_EN          BIT(2)
#define SYSTEM_UART1_CLK_EN         BIT(5)
#define SYSTEM_SPI2_CLK_EN          BIT(6)
#define SYSTEM_I2C_EXT0_CLK_EN      BIT(7)
#define SYSTEM_LEDC_CLK_EN          BIT(11)
#define SYSTEM_TIMERGROUP_CLK_EN    BIT(13)
#define SYSTEM_TIMERGROUP1_CLK_EN   BIT(15)
#define SYSTEM_SYSTIMER_CLK_EN      BIT(29)
#define SYSTEM_CPU_INTR_FROM_CPU_n_REG(n) (DR_REG_SYSTEM_BASE + 0x28 + 4 * (n))

/* ---------------- Interrupt matrix + CPU interrupt controller ---------------- */
#define INTERRUPT_CORE0_MAP_REG(src)        (DR_REG_INTERRUPT_BASE + 4 * (src))
#define INTERRUPT_CORE0_INTR_STATUS_0_REG   (DR_REG_INTERRUPT_BASE + 0xF8)
#define INTERRUPT_CORE0_INTR_STATUS_1_REG   (DR_REG_INTERRUPT_BASE + 0xFC)
#define INTERRUPT_CORE0_CPU_INT_ENABLE_REG  (DR_REG_INTERRUPT_BASE + 0x104)
#define INTERRUPT_CORE0_CPU_INT_TYPE_REG    (DR_REG_INTERRUPT_BASE + 0x108)
#define INTERRUPT_CORE0_CPU_INT_CLEAR_REG   (DR_REG_INTERRUPT_BASE + 0x10C)
#define INTERRUPT_CORE0_CPU_INT_EIP_STATUS_REG (DR_REG_INTERRUPT_BASE + 0x110)
#define INTERRUPT_CORE0_CPU_INT_PRI_n_REG(n) (DR_REG_INTERRUPT_BASE + 0x114 + 4 * (n))
#define INTERRUPT_CORE0_CPU_INT_THRESH_REG  (DR_REG_INTERRUPT_BASE + 0x194)

/* fuentes de interrupcion (numeros de la matriz) */
#define ETS_GPIO_INTR_SOURCE            16
#define ETS_SPI2_INTR_SOURCE            19
#define ETS_UART0_INTR_SOURCE           21
#define ETS_UART1_INTR_SOURCE           22
#define ETS_LEDC_INTR_SOURCE            23
#define ETS_USB_SERIAL_JTAG_INTR_SOURCE 26
#define ETS_RTC_CORE_INTR_SOURCE        27
#define ETS_I2C_EXT0_INTR_SOURCE        29
#define ETS_TG0_T0_LEVEL_INTR_SOURCE    32
#define ETS_TG0_WDT_LEVEL_INTR_SOURCE   33
#define ETS_TG1_T0_LEVEL_INTR_SOURCE    34
#define ETS_TG1_WDT_LEVEL_INTR_SOURCE   35
#define ETS_SYSTIMER_TARGET0_INTR_SOURCE 37
#define ETS_SYSTIMER_TARGET1_INTR_SOURCE 38
#define ETS_SYSTIMER_TARGET2_INTR_SOURCE 39
#define ETS_SHA_INTR_SOURCE             49
#define ETS_FROM_CPU_INTR0_SOURCE       50

/* ---------------- CSRs especificos del ESP32-C3 ---------------- */
#define CSR_PCER_MACHINE    0x7e0
#define CSR_PCMR_MACHINE    0x7e1
#define CSR_PCCR_MACHINE    0x7e2

#define RV_READ_CSR(reg)  ({ uint32_t __v; __asm__ volatile ("csrr %0, " #reg : "=r"(__v)); __v; })
#define RV_WRITE_CSR(reg, val) __asm__ volatile ("csrw " #reg ", %0" :: "rK"(val))
#define RV_SET_CSR(reg, bit) __asm__ volatile ("csrs " #reg ", %0" :: "rK"(bit))
#define RV_CLEAR_CSR(reg, bit) __asm__ volatile ("csrc " #reg ", %0" :: "rK"(bit))

#endif
