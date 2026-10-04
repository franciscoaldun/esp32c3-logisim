/* SPI.h - SPI maestro estilo Arduino sobre SPI2 del modelo (SCK = GPIO6, MOSI = GPIO7, MISO = GPIO2, CS0 = GPIO10).
 * Cada transfer() es una transaccion del hardware: CS0 baja, salen los bits y CS0 vuelve a subir
 * (en la placa, esa subida copia el byte del 74HC595 a sus LEDs). */
#ifndef SPI_H
#define SPI_H
#include "Arduino.h"

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

class SPISettings {
public:
    SPISettings(uint32_t clock = 1000000, uint8_t bitOrder = MSBFIRST, uint8_t dataMode = SPI_MODE0)
        : clock(clock), bitOrder(bitOrder), dataMode(dataMode) {}
    uint32_t clock;
    uint8_t bitOrder, dataMode;
};

class SPIClass {
public:
    void begin(int sck = SCK, int miso = MISO, int mosi = MOSI, int ss = SS);
    void end() {}
    void beginTransaction(const SPISettings &s) { (void)s; }
    void endTransaction() {}
    void setFrequency(uint32_t f) { (void)f; }
    void setDataMode(uint8_t m) { (void)m; }
    void setBitOrder(uint8_t o) { (void)o; }
    uint8_t transfer(uint8_t data);
    uint16_t transfer16(uint16_t data);
    void transfer(void *buf, size_t n);
};
extern SPIClass SPI;
#endif
