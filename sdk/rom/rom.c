/* ROM de arranque: imprime un mensaje, revisa el modo de arranque (pin GPIO9 = BOOT)
 * y salta a la aplicacion en la flash usando el "direct boot" del ESP32-C3:
 * si los primeros 8 bytes de la flash son 0xaedb041d 0xaedb041d, se ejecuta desde 0x42000008. */
#include "esp32c3.h"

static void putc_(char c)
{
    while (UART_TXFIFO_CNT(REG_READ(UART_STATUS_REG(0))) >= 120) { }
    REG_WRITE(UART_FIFO_REG(0), c);
}

static void puts_(const char *s)
{
    while (*s) {
        if (*s == '\n') putc_('\r');
        putc_(*s++);
    }
}

static void puthex(uint32_t v)
{
    char buf[9];
    int i = 8;
    buf[8] = 0;
    do { buf[--i] = "0123456789abcdef"[v & 15]; v >>= 4; } while (v && i);
    puts_(&buf[i]);
}

static const char *reset_name(uint32_t r)
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
    default:   return "?";
    }
}

extern void rom_jump_to_app(uint32_t entry) __attribute__((noreturn));

void rom_main(void)
{
    uint32_t cause = REG_READ(RTC_CNTL_RESET_STATE_REG) & 0x3F;
    uint32_t strap = REG_READ(GPIO_STRAP_REG) & 0xF;
    puts_("\nESP-ROM:esp32c3-logisim (modelo educativo)\nrst:0x");
    puthex(cause);
    puts_(" (");
    puts_(reset_name(cause));
    puts_("),boot:0x");
    puthex(strap);
    if (!(strap & 0x8)) {
        /* GPIO9 en 0 al arrancar: modo descarga (en el chip real espera a esptool) */
        puts_(" (DOWNLOAD(UART0))\nwaiting for download\n");
        for (;;) __asm__ volatile ("wfi");
    }
    puts_(" (SPI_FAST_FLASH_BOOT)\n");
    volatile uint32_t *flash = (volatile uint32_t *)SOC_IROM_LOW;
    for (;;) {
        uint32_t w0 = flash[0], w1 = flash[1];
        if ((w0 & 0xFF) == 0xE9) {
            puts_("formato de imagen ESP-IDF no soportado por el modelo: usa direct boot\n");
        } else if (w0 == 0xAEDB041D && w1 == 0xAEDB041D) {
            rom_jump_to_app(SOC_IROM_LOW + 8);
        } else {
            puts_("invalid header: 0x");
            puthex(w0);
            puts_("\n");
        }
        for (volatile int i = 0; i < 2000; i++) { }
    }
}

void rom_exception(uint32_t mcause, uint32_t mepc, uint32_t mtval)
{
    puts_("\nGuru Meditation Error (ROM): mcause=0x");
    puthex(mcause);
    puts_(" mepc=0x");
    puthex(mepc);
    puts_(" mtval=0x");
    puthex(mtval);
    puts_("\n");
    for (;;) __asm__ volatile ("wfi");
}
