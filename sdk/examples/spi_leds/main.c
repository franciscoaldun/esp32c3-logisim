/* SPI2 + registro de desplazamiento 74HC595: la placa tiene un 74HC595 conectado a
 * SCK = GPIO6, MOSI = GPIO7 y CS0 = GPIO10 (la subida de CS0 copia el dato a sus salidas).
 * Sus 8 salidas encienden la barra de LEDs. */
#include "sdk.h"

static void mostrar(uint8_t v)
{
    uint8_t rx;
    spi_transfer(&v, &rx, 1);
}

int main(void)
{
    printf("\nSPI2 -> 74HC595 -> barra de 8 LEDs\n");
    spi_init(2);                                /* SCK: 2 ciclos en alto, 2 en bajo */
    printf("Contador binario:\n");
    for (int i = 0; i <= 16; i++) {
        mostrar(i);
        printf("%d ", i);
        delay_ticks(300);
    }
    printf("\nAuto fantastico:\n");
    for (;;) {
        for (int i = 0; i < 7; i++) { mostrar(1 << i); delay_ticks(150); }
        for (int i = 7; i > 0; i--) { mostrar(1 << i); delay_ticks(150); }
    }
}
