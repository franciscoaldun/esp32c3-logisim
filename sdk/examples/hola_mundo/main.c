/* Hola mundo: imprime informacion del chip y repite lo que escribas en el teclado. */
#include "sdk.h"

int main(void)
{
    uint8_t mac[6];
    read_mac(mac);
    printf("\nHola desde el ESP32-C3 en Logisim!\n");
    printf("Motivo del reset: %s\n", reset_reason_name(reset_reason()));
    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("misa=0x%08x  mvendorid=0x%x\n", RV_READ_CSR(misa), RV_READ_CSR(mvendorid));
    int a = 123456, b = 789;
    printf("%d * %d = %d ; %d / %d = %d\n", a, b, a * b, a, b, a / b);
    printf("Escribe algo en el teclado:\n");
    for (;;) {
        int c = uart_getc(0);
        if (c >= 0) {
            if (c == '\r') c = '\n';
            putchar_((char)c);
        }
    }
}
