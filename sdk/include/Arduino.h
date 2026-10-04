/*
 * Arduino.h - capa "estilo Arduino" para el ESP32-C3 del modelo Logisim.
 *
 * Tu sketch (.ino) se compila tal cual: setup() se llama una vez y loop() para siempre.
 * Todo se traduce a los registros reales del chip simulado (GPIO, LEDC, UART0, I2C, SPI2, SYSTIMER).
 *
 * Diferencias con el Arduino de verdad:
 *  - El tiempo va en camara lenta: 1 ms del programa = SIM_CICLOS_POR_MS ciclos del modelo (1 por defecto).
 *  - Serial.begin() acepta cualquier velocidad; la UART del modelo envia un caracter cada 40 ciclos.
 *  - analogRead() devuelve 0 (el modelo no tiene ADC). analogWrite() usa el periferico LEDC (PWM).
 *  - No hay WiFi ni Bluetooth (la radio no es logica digital simulable).
 */
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "sdk.h"

#define HIGH 0x1
#define LOW  0x0

#define INPUT             0x01
#define OUTPUT            0x03
#define INPUT_PULLUP      0x05
#define INPUT_PULLDOWN    0x09
#define OUTPUT_OPEN_DRAIN 0x13

#define RISING  0x01
#define FALLING 0x02
#define CHANGE  0x03
#define ONLOW   0x04
#define ONHIGH  0x05

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

/* pines de la placa del modelo */
#define LED_BUILTIN 8           /* LED rojo */
#define BOOT_PIN    9           /* boton BOOT (vale 0 al presionarlo) */
#define SDA         4           /* memoria I2C */
#define SCL         5
#define SCK         6           /* SPI2 -> 74HC595 */
#define MOSI        7
#define MISO        2
#define SS          10

#define PI         3.1415926535897932384626433832795
#define HALF_PI    1.5707963267948966192313216916398
#define TWO_PI     6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

typedef uint8_t byte;
typedef bool boolean;
typedef unsigned int word;

#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit)          ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b)                         (1UL << (b))
#define lowByte(w)                     ((uint8_t)((w) & 0xff))
#define highByte(w)                    ((uint8_t)((w) >> 8))
#define constrain(amt, low, high)      ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define sq(x)                          ((x) * (x))
#define radians(deg)                   ((deg) * DEG_TO_RAD)
#define degrees(rad)                   ((rad) * RAD_TO_DEG)
#define digitalPinToInterrupt(p)       (p)

#ifdef __cplusplus
template <class T, class L> static inline auto min(const T &a, const L &b) -> decltype((b < a) ? b : a) { return (b < a) ? b : a; }
template <class T, class L> static inline auto max(const T &a, const L &b) -> decltype((b < a) ? b : a) { return (a < b) ? b : a; }
extern "C" {
#endif

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
void analogWrite(uint8_t pin, int value);       /* 0..255 con el LEDC */
int analogRead(uint8_t pin);                    /* siempre 0: el modelo no tiene ADC */
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void attachInterrupt(uint8_t pin, void (*isr)(void), int mode);
void detachInterrupt(uint8_t pin);
void interrupts(void);
void noInterrupts(void);
void yield(void);
void randomSeed(unsigned long seed);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
#define LSBFIRST 0
#define MSBFIRST 1

void setup(void);
void loop(void);

#ifdef __cplusplus
}

long random(long howbig);
long random(long howsmall, long howbig);

/* ------------------------------------------------------------------ String (version reducida) */
class String {
public:
    String(const char *s = "");
    String(const String &s);
    explicit String(char c);
    String(int v, unsigned char base = 10);
    String(unsigned int v, unsigned char base = 10);
    String(long v, unsigned char base = 10);
    String(unsigned long v, unsigned char base = 10);
    String(double v, unsigned char decimals = 2);
    ~String();
    String &operator=(const String &s);
    String &operator=(const char *s);
    String &operator+=(const String &s) { concat(s.c_str()); return *this; }
    String &operator+=(const char *s) { concat(s); return *this; }
    String &operator+=(char c) { char b[2] = {c, 0}; concat(b); return *this; }
    String &operator+=(int v) { return *this += String(v); }
    String &operator+=(unsigned int v) { return *this += String(v); }
    String &operator+=(long v) { return *this += String(v); }
    String &operator+=(unsigned long v) { return *this += String(v); }
    String &operator+=(double v) { return *this += String(v); }
    bool concat(const char *s);
    bool concat(const String &s) { return concat(s.c_str()); }
    unsigned int length() const { return len; }
    const char *c_str() const { return buf; }
    char charAt(unsigned int i) const { return i < len ? buf[i] : 0; }
    char operator[](unsigned int i) const { return charAt(i); }
    void setCharAt(unsigned int i, char c) { if (i < len) buf[i] = c; }
    bool equals(const String &s) const { return strcmp(buf, s.buf) == 0; }
    bool equalsIgnoreCase(const String &s) const;
    bool operator==(const String &s) const { return equals(s); }
    bool operator==(const char *s) const { return strcmp(buf, s) == 0; }
    bool operator!=(const String &s) const { return !equals(s); }
    bool operator!=(const char *s) const { return strcmp(buf, s) != 0; }
    bool operator<(const String &s) const { return strcmp(buf, s.buf) < 0; }
    bool operator>(const String &s) const { return strcmp(buf, s.buf) > 0; }
    int compareTo(const String &s) const { return strcmp(buf, s.buf); }
    bool startsWith(const String &s) const { return strncmp(buf, s.buf, s.len) == 0; }
    bool endsWith(const String &s) const { return s.len <= len && strcmp(buf + len - s.len, s.buf) == 0; }
    int indexOf(char c, unsigned int from = 0) const;
    int indexOf(const String &s, unsigned int from = 0) const;
    int lastIndexOf(char c) const;
    String substring(unsigned int from, unsigned int to = 0xFFFFFFFF) const;
    void replace(const String &find, const String &repl);
    void remove(unsigned int index, unsigned int count = 0xFFFFFFFF);
    void toUpperCase();
    void toLowerCase();
    void trim();
    long toInt() const { return atol(buf); }
    float toFloat() const { return (float)atof(buf); }
    double toDouble() const { return atof(buf); }
    bool isEmpty() const { return len == 0; }
    void reserve(unsigned int n) { grow(n); }
    operator bool() const { return true; }
private:
    char *buf;
    unsigned int len, cap;
    void grow(unsigned int n);
    void set(const char *s);
};
String operator+(const String &a, const String &b);
String operator+(const String &a, const char *b);
String operator+(const char *a, const String &b);
String operator+(const String &a, char b);
String operator+(const String &a, int b);
String operator+(const String &a, unsigned int b);
String operator+(const String &a, long b);
String operator+(const String &a, unsigned long b);
String operator+(const String &a, double b);

/* ------------------------------------------------------------------ Serial (UART0 -> terminal de la placa) */
class HardwareSerial {
public:
    void begin(unsigned long baud) { (void)baud; }
    void begin(unsigned long baud, uint32_t config, int rx = -1, int tx = -1) { (void)baud; (void)config; (void)rx; (void)tx; }
    void end() {}
    int available();
    int read();
    int peek();
    void flush() {}
    size_t write(uint8_t c);
    size_t write(const char *s);
    size_t write(const uint8_t *b, size_t n);
    size_t print(const char *s) { return write(s); }
    size_t print(const String &s) { return write(s.c_str()); }
    size_t print(char c) { return write((uint8_t)c); }
    size_t print(int v, int base = DEC) { return print((long)v, base); }
    size_t print(unsigned int v, int base = DEC) { return print((unsigned long)v, base); }
    size_t print(long v, int base = DEC);
    size_t print(unsigned long v, int base = DEC);
    size_t print(long long v, int base = DEC);
    size_t print(unsigned long long v, int base = DEC);
    size_t print(double v, int digits = 2);
    size_t println() { return write("\r\n"); }
    template <class T> size_t println(const T &v) { size_t n = print(v); return n + println(); }
    template <class T> size_t println(const T &v, int fmt) { size_t n = print(v, fmt); return n + println(); }
    size_t printf(const char *fmt, ...);
    String readString();
    String readStringUntil(char terminator);
    long parseInt();
    void setTimeout(unsigned long ms) { timeout = ms; }
    operator bool() const { return true; }
private:
    unsigned long timeout = 1000;
    int peeked = -1;
};
extern HardwareSerial Serial;

#endif /* __cplusplus */
#endif
