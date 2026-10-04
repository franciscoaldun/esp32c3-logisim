/* I2C: la placa tiene una memoria 24C02 (direccion 0x50) en GPIO4 = SDA y GPIO5 = SCL.
 * Guarda un texto, lo vuelve a leer, y cuenta los reinicios en la memoria (que esta fuera del chip,
 * asi que conserva el valor cuando presionas RESET). */
#include "sdk.h"

#define EEPROM 0x50

int main(void)
{
    printf("\nI2C: memoria 24C02 en la direccion 0x%x\n", EEPROM);
    i2c_init(4, 5);

    uint8_t pos = 0x00, cuenta;
    if (i2c_write_read(EEPROM, &pos, 1, &cuenta, 1) < 0) {
        printf("la memoria no responde (NACK)\n");
        for (;;) { }
    }
    cuenta++;
    uint8_t w[2] = {0x00, cuenta};
    i2c_write(EEPROM, w, 2);
    printf("Este chip arranco %d veces (contador guardado en la memoria I2C)\n", cuenta);

    const char *texto = "Hola I2C!";
    uint8_t buf[16];
    buf[0] = 0x10;                                    /* direccion dentro de la memoria */
    int n = 0;
    while (texto[n]) { buf[1 + n] = texto[n]; n++; }
    i2c_write(EEPROM, buf, n + 1);
    printf("Escribi \"%s\" en la posicion 0x10\n", texto);

    uint8_t leido[16];
    i2c_write_read(EEPROM, &buf[0], 1, leido, n);
    leido[n] = 0;
    printf("Lei: \"%s\"\n", (char *)leido);

    printf("Busco dispositivos en el bus:");
    for (int a = 0x48; a <= 0x58; a++) {
        if (i2c_write(a, 0, 0) == 0) printf(" 0x%x", a);
    }
    printf("\nPresiona RESET para ver como sube el contador.\n");
    for (;;) { }
}
