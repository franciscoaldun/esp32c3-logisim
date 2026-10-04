/* Wire.h - I2C maestro estilo Arduino sobre el periferico I2C del modelo (SDA = GPIO4, SCL = GPIO5) */
#ifndef WIRE_H
#define WIRE_H
#include "Arduino.h"

class TwoWire {
public:
    bool begin(int sda = SDA, int scl = SCL, uint32_t freq = 0);
    void end() {}
    void setClock(uint32_t freq) { (void)freq; }
    void beginTransmission(uint8_t address);
    uint8_t endTransmission(bool sendStop = true);   /* 0 = ok, 2 = el dispositivo no respondio */
    size_t write(uint8_t b);
    size_t write(const uint8_t *data, size_t n);
    size_t requestFrom(uint8_t address, size_t n, bool sendStop = true);
    int available() { return rx_len - rx_pos; }
    int read() { return rx_pos < rx_len ? rx_buf[rx_pos++] : -1; }
    int peek() { return rx_pos < rx_len ? rx_buf[rx_pos] : -1; }
private:
    uint8_t addr = 0;
    uint8_t tx_buf[32];
    int tx_len = 0;
    bool pending = false;        /* endTransmission(false): los bytes se mandan junto con la lectura */
    uint8_t rx_buf[32];
    int rx_len = 0, rx_pos = 0;
};
extern TwoWire Wire;
#endif
