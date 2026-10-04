/* sdk.c - implementacion del mini SDK */
#include "sdk.h"

/* ---------------- consola ---------------- */
void uart_putc(int u, char c)
{
    while (UART_TXFIFO_CNT(REG_READ(UART_STATUS_REG(u))) >= 120) { }
    REG_WRITE(UART_FIFO_REG(u), (uint8_t)c);
}

void sdk_uart_putc(int u, char c) { uart_putc(u, c); }

void uart_puts(int u, const char *s)
{
    int n = 0;
    while (s[n]) n++;
    uart_write_n(u, s, n, 1);
}

int uart_available(int u) { return UART_RXFIFO_CNT(REG_READ(UART_STATUS_REG(u))); }

int uart_getc(int u)
{
    if (UART_RXFIFO_CNT(REG_READ(UART_STATUS_REG(u))) == 0) return -1;
    return REG_READ(UART_FIFO_REG(u)) & 0xFF;
}

int sdk_uart_getc(int u) { return uart_getc(u); }

int uart_getc_wait(int u)
{
    int c;
    while ((c = uart_getc(u)) < 0) { }
    return c;
}

/* ---------------- tiempo ---------------- */
uint64_t systimer_ticks(void)
{
    REG_WRITE(SYSTIMER_UNIT0_OP_REG, SYSTIMER_TIMER_UNIT0_UPDATE);
    while (!(REG_READ(SYSTIMER_UNIT0_OP_REG) & SYSTIMER_TIMER_UNIT0_VALUE_VALID)) { }
    uint32_t lo = REG_READ(SYSTIMER_UNIT0_VALUE_LO_REG);
    uint32_t hi = REG_READ(SYSTIMER_UNIT0_VALUE_HI_REG);
    return ((uint64_t)hi << 32) | lo;
}

void delay_ticks(uint32_t t)
{
    uint32_t start = (uint32_t)systimer_ticks();
    while ((uint32_t)systimer_ticks() - start < t) { }
}

uint32_t sdk_millis(void) { return (uint32_t)(systimer_ticks() / SIM_CICLOS_POR_MS); }
uint64_t sdk_micros(void) { return systimer_ticks() * 1000 / SIM_CICLOS_POR_MS; }
void sdk_delay_ms(uint32_t ms) { delay_ticks(ms * SIM_CICLOS_POR_MS); }
void sdk_delay_us(uint32_t us) { delay_ticks((uint32_t)((uint64_t)us * SIM_CICLOS_POR_MS / 1000)); }

/* ---------------- GPIO ---------------- */
void gpio_output(int pin)
{
    REG_WRITE(IO_MUX_GPIO_REG(pin), (REG_READ(IO_MUX_GPIO_REG(pin)) & ~MCU_SEL_M) | (PIN_FUNC_GPIO << MCU_SEL_S) | FUN_IE);
    REG_WRITE(GPIO_FUNC_OUT_SEL_CFG_REG(pin), SIG_GPIO_OUT_IDX);
    REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(pin));
}

void gpio_input(int pin, int pullup)
{
    uint32_t v = (REG_READ(IO_MUX_GPIO_REG(pin)) & ~(MCU_SEL_M | FUN_PU | FUN_PD)) | (PIN_FUNC_GPIO << MCU_SEL_S) | FUN_IE;
    if (pullup) v |= FUN_PU;
    REG_WRITE(IO_MUX_GPIO_REG(pin), v);
    REG_WRITE(GPIO_ENABLE_W1TC_REG, BIT(pin));
}

void gpio_input_pull(int pin, int pullup, int pulldown)
{
    gpio_input(pin, pullup);
    if (pulldown) REG_SET_BIT(IO_MUX_GPIO_REG(pin), FUN_PD);
}

void gpio_set(int pin, int v) { REG_WRITE(v ? GPIO_OUT_W1TS_REG : GPIO_OUT_W1TC_REG, BIT(pin)); }
void gpio_toggle(int pin) { REG_WRITE(GPIO_OUT_REG, REG_READ(GPIO_OUT_REG) ^ BIT(pin)); }
int  gpio_get(int pin) { return (REG_READ(GPIO_IN_REG) >> pin) & 1; }

/* interrupciones por pin: un solo manejador para la fuente GPIO (linea 31 de la CPU) que reparte por pin */
#define GPIO_CPU_LINE 31
static void (*pin_handlers[NUM_GPIO])(void *);
static void *pin_args[NUM_GPIO];

static void gpio_dispatch(void *arg)
{
    (void)arg;
    uint32_t st = REG_READ(GPIO_STATUS_REG);
    REG_WRITE(GPIO_STATUS_W1TC_REG, st);
    for (int p = 0; p < NUM_GPIO; p++)
        if ((st & BIT(p)) && pin_handlers[p]) pin_handlers[p](pin_args[p]);
}

void gpio_pin_isr(int pin, int type, void (*h)(void *), void *arg)
{
    static int installed;
    pin_handlers[pin] = type ? h : 0;
    pin_args[pin] = arg;
    REG_WRITE(GPIO_PIN_REG(pin), (REG_READ(GPIO_PIN_REG(pin)) & ~(0x7u << GPIO_PIN_INT_TYPE_S | 0x1Fu << GPIO_PIN_INT_ENA_S))
              | ((uint32_t)type << GPIO_PIN_INT_TYPE_S) | (type ? 1u << GPIO_PIN_INT_ENA_S : 0));
    REG_WRITE(GPIO_STATUS_W1TC_REG, BIT(pin));
    if (!installed) {
        installed = 1;
        intr_attach(ETS_GPIO_INTR_SOURCE, GPIO_CPU_LINE, 3, gpio_dispatch, 0);
        intr_global_enable();
    }
}

/* ---------------- matriz GPIO / IO MUX ---------------- */
void gpio_matrix_out(int pin, int signal)
{
    REG_WRITE(IO_MUX_GPIO_REG(pin), (REG_READ(IO_MUX_GPIO_REG(pin)) & ~MCU_SEL_M) | (PIN_FUNC_GPIO << MCU_SEL_S) | FUN_IE);
    REG_WRITE(GPIO_FUNC_OUT_SEL_CFG_REG(pin), signal);
    REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(pin));
}

void gpio_iomux_func(int pin, int func)
{
    REG_WRITE(IO_MUX_GPIO_REG(pin), (REG_READ(IO_MUX_GPIO_REG(pin)) & ~MCU_SEL_M) | (func << MCU_SEL_S) | FUN_IE);
}

void periph_enable(uint32_t bit)
{
    REG_SET_BIT(SYSTEM_PERIP_CLK_EN0_REG, bit);
    REG_CLR_BIT(SYSTEM_PERIP_RST_EN0_REG, bit);
}

/* ---------------- LEDC ---------------- */
void sdk_ledc_timer(int timer, int bits, int divisor)
{
    periph_enable(SYSTEM_LEDC_CLK_EN);
    REG_WRITE(LEDC_CONF_REG, LEDC_CLK_EN | (1 << LEDC_APB_CLK_SEL_S));        /* reloj: APB */
    REG_WRITE(LEDC_TIMERx_CONF_REG(timer), (bits << LEDC_TIMER_DUTY_RES_S) | ((divisor << 8) << LEDC_TIMER_CLK_DIV_S)
              | LEDC_TIMER_PARA_UP);                                          /* sin LEDC_TIMER_RST: corre */
}

static void ledc_update(int ch, uint32_t conf1)
{
    REG_WRITE(LEDC_CHn_CONF1_REG(ch), conf1);
    REG_SET_BIT(LEDC_CHn_CONF0_REG(ch), LEDC_PARA_UP);
}

void sdk_ledc_channel(int ch, int timer, int pin, uint32_t duty)
{
    REG_WRITE(LEDC_CHn_HPOINT_REG(ch), 0);
    REG_WRITE(LEDC_CHn_DUTY_REG(ch), duty << 4);                 /* 4 bits de fraccion */
    REG_WRITE(LEDC_CHn_CONF0_REG(ch), (timer << LEDC_TIMER_SEL_S) | LEDC_SIG_OUT_EN);
    ledc_update(ch, LEDC_DUTY_START | LEDC_DUTY_INC | (1 << LEDC_DUTY_NUM_S) | (1 << LEDC_DUTY_CYCLE_S));
    gpio_matrix_out(pin, LEDC_LS_SIG_OUT0_IDX + ch);
}

void sdk_ledc_duty(int ch, uint32_t duty)
{
    REG_WRITE(LEDC_CHn_DUTY_REG(ch), duty << 4);
    ledc_update(ch, LEDC_DUTY_START | LEDC_DUTY_INC | (1 << LEDC_DUTY_NUM_S) | (1 << LEDC_DUTY_CYCLE_S));
}

void sdk_ledc_fade_from(int ch, uint32_t start, int up, int steps, int periods, int step)
{
    REG_WRITE(LEDC_CHn_DUTY_REG(ch), start << 4);
    ledc_update(ch, LEDC_DUTY_START | (up ? LEDC_DUTY_INC : 0) | (steps << LEDC_DUTY_NUM_S)
                | (periods << LEDC_DUTY_CYCLE_S) | (step << LEDC_DUTY_SCALE_S));
}

void sdk_ledc_fade(int ch, int up, int steps, int periods, int step)
{
    sdk_ledc_fade_from(ch, sdk_ledc_duty_now(ch), up, steps, periods, step);
}

uint32_t sdk_ledc_duty_now(int ch) { return REG_READ(LEDC_CHn_DUTY_R_REG(ch)) >> 4; }

/* ---------------- SPI2 ---------------- */
void spi_init(int half)
{
    periph_enable(SYSTEM_SPI2_CLK_EN);
    gpio_iomux_func(6, PIN_FUNC_FSPI);          /* SCK  */
    gpio_iomux_func(7, PIN_FUNC_FSPI);          /* MOSI */
    gpio_iomux_func(10, PIN_FUNC_FSPI);         /* CS0  */
    gpio_iomux_func(2, PIN_FUNC_FSPI);          /* MISO */
    if (half <= 1) {
        REG_WRITE(SPI_CLOCK_REG, SPI_CLK_EQU_SYSCLK);
    } else {
        int n = 2 * half - 1;                   /* (CLKCNT_N + 1) = ciclos por periodo de SCK */
        REG_WRITE(SPI_CLOCK_REG, (n << SPI_CLKCNT_N_S) | ((n / 2) << SPI_CLKCNT_H_S) | (n << SPI_CLKCNT_L_S));
    }
    REG_WRITE(SPI_USER_REG, SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN);   /* full duplex, modo 0 */
    REG_WRITE(SPI_MISC_REG, 0x3E);              /* solo CS0 activo */
    REG_WRITE(SPI_CMD_REG, SPI_UPDATE);
}

void spi_transfer(const uint8_t *tx, uint8_t *rx, int n)
{
    for (int i = 0; i < (n + 3) / 4; i++) {
        uint32_t w = 0;
        for (int b = 0; b < 4; b++)
            if (4 * i + b < n) w |= (uint32_t)tx[4 * i + b] << (8 * b);
        REG_WRITE(SPI_W_REG(i), w);
    }
    REG_WRITE(SPI_MS_DLEN_REG, 8 * n - 1);
    REG_WRITE(SPI_CMD_REG, SPI_UPDATE);
    REG_WRITE(SPI_CMD_REG, SPI_USR);
    while (REG_READ(SPI_CMD_REG) & SPI_USR) { }
    if (rx) {
        for (int i = 0; i < n; i++) rx[i] = REG_READ(SPI_W_REG(i / 4)) >> (8 * (i % 4));
    }
}

/* ---------------- I2C ---------------- */
void i2c_init(int sda, int scl)
{
    periph_enable(SYSTEM_I2C_EXT0_CLK_EN);
    int pins[2] = {sda, scl};
    for (int i = 0; i < 2; i++) {
        int p = pins[i];
        REG_WRITE(GPIO_PIN_REG(p), GPIO_PIN_PAD_DRIVER);                 /* salida de drenador abierto */
        REG_WRITE(IO_MUX_GPIO_REG(p), (PIN_FUNC_GPIO << MCU_SEL_S) | FUN_IE | FUN_PU);
        REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(p));
    }
    REG_WRITE(GPIO_FUNC_OUT_SEL_CFG_REG(sda), I2CEXT0_SDA_IDX);
    REG_WRITE(GPIO_FUNC_OUT_SEL_CFG_REG(scl), I2CEXT0_SCL_IDX);
    REG_WRITE(GPIO_FUNC_IN_SEL_CFG_REG(I2CEXT0_SDA_IDX), GPIO_SIG_IN_SEL | sda);
    REG_WRITE(GPIO_FUNC_IN_SEL_CFG_REG(I2CEXT0_SCL_IDX), GPIO_SIG_IN_SEL | scl);
    REG_WRITE(I2C_CTR_REG, I2C_MS_MODE | BIT(8) | BIT(3) | BIT(1) | BIT(0));  /* maestro */
    REG_WRITE(I2C_SCL_LOW_PERIOD_REG, 3);       /* en el modelo: cada cuarto de bit dura 3 + 1 ciclos */
    REG_WRITE(I2C_SCL_HIGH_PERIOD_REG, 1);
    REG_SET_BIT(I2C_CTR_REG, I2C_CONF_UPGATE);
}

static int i2c_run(int ncmds, const uint32_t *cmds)
{
    for (int i = 0; i < ncmds; i++) REG_WRITE(I2C_COMD_REG(i), cmds[i]);
    REG_WRITE(I2C_INT_CLR_REG, 0xFFFFFFFF);
    REG_SET_BIT(I2C_CTR_REG, I2C_CONF_UPGATE);
    REG_SET_BIT(I2C_CTR_REG, I2C_TRANS_START);
    for (;;) {
        uint32_t r = REG_READ(I2C_INT_RAW_REG);
        if (r & I2C_NACK_INT) return -1;
        if (r & I2C_TRANS_COMPLETE_INT) return 0;
    }
}

static void i2c_fifo_reset(void)
{
    REG_SET_BIT(I2C_FIFO_CONF_REG, I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
    REG_CLR_BIT(I2C_FIFO_CONF_REG, I2C_TX_FIFO_RST | I2C_RX_FIFO_RST);
}

int i2c_write(int addr, const uint8_t *data, int n)
{
    i2c_fifo_reset();
    REG_WRITE(I2C_DATA_REG, addr << 1);
    for (int i = 0; i < n; i++) REG_WRITE(I2C_DATA_REG, data[i]);
    uint32_t cmds[3] = {
        I2C_COMMAND(I2C_CMD_RSTART, 0, 0, 0, 0),
        I2C_COMMAND(I2C_CMD_WRITE, 0, 0, 1, n + 1),
        I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0),
    };
    int r = i2c_run(3, cmds);
    if (r < 0) {                                  /* sin respuesta: libera el bus con un STOP */
        uint32_t stop[1] = {I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0)};
        i2c_run(1, stop);
    }
    return r;
}

int i2c_read(int addr, uint8_t *rd, int rn)
{
    i2c_fifo_reset();
    REG_WRITE(I2C_DATA_REG, (addr << 1) | 1);
    uint32_t cmds[5];
    int k = 0;
    cmds[k++] = I2C_COMMAND(I2C_CMD_RSTART, 0, 0, 0, 0);
    cmds[k++] = I2C_COMMAND(I2C_CMD_WRITE, 0, 0, 1, 1);
    if (rn > 1) cmds[k++] = I2C_COMMAND(I2C_CMD_READ, 0, 0, 0, rn - 1);
    cmds[k++] = I2C_COMMAND(I2C_CMD_READ, 1, 0, 0, 1);
    cmds[k++] = I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0);
    int r = i2c_run(k, cmds);
    if (r < 0) {
        uint32_t stop[1] = {I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0)};
        i2c_run(1, stop);
        return r;
    }
    for (int i = 0; i < rn; i++) rd[i] = REG_READ(I2C_DATA_REG);
    return 0;
}

int i2c_write_read(int addr, const uint8_t *w, int wn, uint8_t *rd, int rn)
{
    i2c_fifo_reset();
    REG_WRITE(I2C_DATA_REG, addr << 1);
    for (int i = 0; i < wn; i++) REG_WRITE(I2C_DATA_REG, w[i]);
    REG_WRITE(I2C_DATA_REG, (addr << 1) | 1);
    uint32_t cmds[7];
    int k = 0;
    cmds[k++] = I2C_COMMAND(I2C_CMD_RSTART, 0, 0, 0, 0);
    cmds[k++] = I2C_COMMAND(I2C_CMD_WRITE, 0, 0, 1, wn + 1);
    cmds[k++] = I2C_COMMAND(I2C_CMD_RSTART, 0, 0, 0, 0);
    cmds[k++] = I2C_COMMAND(I2C_CMD_WRITE, 0, 0, 1, 1);
    if (rn > 1) cmds[k++] = I2C_COMMAND(I2C_CMD_READ, 0, 0, 0, rn - 1);   /* ACK a cada byte */
    cmds[k++] = I2C_COMMAND(I2C_CMD_READ, 1, 0, 0, 1);                    /* NACK al ultimo */
    cmds[k++] = I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0);
    int r = i2c_run(k, cmds);
    if (r < 0) {
        uint32_t stop[1] = {I2C_COMMAND(I2C_CMD_STOP, 0, 0, 0, 0)};
        i2c_run(1, stop);
        return r;
    }
    for (int i = 0; i < rn; i++) rd[i] = REG_READ(I2C_DATA_REG);
    return 0;
}

/* ---------------- interrupciones ---------------- */
static irq_handler_t handlers[32];
static void *handler_args[32];

void intr_attach(int source, int line, int prio, irq_handler_t h, void *arg)
{
    handlers[line] = h;
    handler_args[line] = arg;
    REG_WRITE(INTERRUPT_CORE0_MAP_REG(source), line);
    REG_WRITE(INTERRUPT_CORE0_CPU_INT_PRI_n_REG(line), prio);
    REG_CLR_BIT(INTERRUPT_CORE0_CPU_INT_TYPE_REG, BIT(line));      /* por nivel */
    REG_SET_BIT(INTERRUPT_CORE0_CPU_INT_ENABLE_REG, BIT(line));
}

void intr_global_enable(void)  { RV_SET_CSR(mstatus, 8); }
void intr_global_disable(void) { RV_CLEAR_CSR(mstatus, 8); }

void sdk_irq_dispatch(uint32_t mcause)
{
    uint32_t line = mcause & 31;
    if (handlers[line]) handlers[line](handler_args[line]);
}

static const char *exc_name(uint32_t c)
{
    switch (c) {
    case 1: return "Instruction access fault";
    case 2: return "Illegal instruction";
    case 3: return "Breakpoint";
    case 4: return "Load address misaligned";
    case 5: return "Load access fault";
    case 6: return "Store address misaligned";
    case 7: return "Store access fault";
    case 11: return "Environment call (ecall)";
    default: return "?";
    }
}

void sdk_panic(uint32_t *regs, uint32_t mcause, uint32_t mepc, uint32_t mtval)
{
    static const char *names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3",
        "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
    printf("\nGuru Meditation Error: Core 0 panic'ed (%s). Exception was unhandled.\n", exc_name(mcause));
    printf("MEPC    : 0x%08x  MCAUSE  : 0x%08x  MTVAL   : 0x%08x\n", mepc, mcause, mtval);
    for (int i = 1; i < 32; i++) {
        printf("%s%s: 0x%08x%s", names[i], (names[i][2] ? " " : "  "), regs[i], (i % 4 == 3) ? "\n" : "  ");
    }
    printf("\n");
    __asm__ volatile ("ebreak");
    for (;;) { }
}

/* ---------------- chip ---------------- */
uint32_t reset_reason(void) { return REG_READ(RTC_CNTL_RESET_STATE_REG) & 0x3F; }

const char *reset_reason_name(uint32_t r)
{
    switch (r) {
    case 0x01: return "POWERON";
    case 0x03: return "RTC_SW_SYS_RST";
    case 0x07: return "TG0WDT_SYS_RST";
    case 0x08: return "TG1WDT_SYS_RST";
    case 0x09: return "RTCWDT_SYS_RST";
    case 0x0B: return "TG0WDT_CPU_RST";
    case 0x0C: return "RTC_SW_CPU_RST";
    case 0x0D: return "RTCWDT_CPU_RST";
    case 0x10: return "RTCWDT_RTC_RST";
    case 0x11: return "TG1WDT_CPU_RST";
    default: return "?";
    }
}

void read_mac(uint8_t mac[6])
{
    uint32_t lo = REG_READ(EFUSE_RD_MAC_SPI_SYS_0_REG), hi = REG_READ(EFUSE_RD_MAC_SPI_SYS_1_REG);
    mac[0] = hi >> 8; mac[1] = hi; mac[2] = lo >> 24; mac[3] = lo >> 16; mac[4] = lo >> 8; mac[5] = lo;
}

void watchdogs_disable(void)
{
    for (int i = 0; i < 2; i++) {
        REG_WRITE(TIMG_WDTWPROTECT_REG(i), TIMG_WDT_WKEY_VALUE);
        REG_WRITE(TIMG_WDTCONFIG0_REG(i), 0);
        REG_WRITE(TIMG_WDTWPROTECT_REG(i), 0);
    }
    REG_WRITE(RTC_CNTL_WDTWPROTECT_REG, RTC_CNTL_WDT_WKEY_VALUE);
    REG_WRITE(RTC_CNTL_WDTCONFIG0_REG, 0);
    REG_WRITE(RTC_CNTL_WDTWPROTECT_REG, 0);
    REG_WRITE(RTC_CNTL_SWD_WPROTECT_REG, RTC_CNTL_SWD_WKEY_VALUE);
    REG_SET_BIT(RTC_CNTL_SWD_CONF_REG, BIT(31));     /* super watchdog: auto-alimentado */
    REG_WRITE(RTC_CNTL_SWD_WPROTECT_REG, 0);
}

void system_reset(void) { REG_WRITE(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_SW_SYS_RST); for (;;) { } }

uint32_t random32(void) { return REG_READ(WDEV_RND_REG); }

void sdk_init(void)
{
    watchdogs_disable();
    REG_WRITE(INTERRUPT_CORE0_CPU_INT_THRESH_REG, 1);
}
