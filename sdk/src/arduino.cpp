/* arduino.cpp - implementacion de la capa estilo Arduino (ver include/Arduino.h) */
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"

/* ------------------------------------------------------------------ pines */
static int8_t pwm_channel[NUM_GPIO] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static int pwm_used;

extern "C" void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= NUM_GPIO) return;
    pwm_channel[pin] = -1;                          /* si estaba en PWM, vuelve a ser un GPIO */
    switch (mode) {
    case OUTPUT:
    case OUTPUT_OPEN_DRAIN: gpio_output(pin); break;
    case INPUT_PULLUP: gpio_input(pin, 1); break;
    case INPUT_PULLDOWN: gpio_input_pull(pin, 0, 1); break;
    default: gpio_input(pin, 0); break;
    }
}

extern "C" void digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin >= NUM_GPIO) return;
    if (pwm_channel[pin] >= 0) {                    /* el pin estaba en PWM: vuelve a ser un GPIO normal */
        gpio_output(pin);
        pwm_channel[pin] = -1;
    }
    gpio_set(pin, val);
}

extern "C" int digitalRead(uint8_t pin) { return pin < NUM_GPIO ? gpio_get(pin) : 0; }

/* analogWrite: PWM de 8 bits con el LEDC (timer 0, periodo = 256 ciclos), un canal por pin (6 canales) */
static int8_t pin_of_channel[6] = {-1, -1, -1, -1, -1, -1};

extern "C" void analogWrite(uint8_t pin, int value)
{
    if (pin >= NUM_GPIO) return;
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    uint32_t duty = value == 255 ? 256 : (uint32_t)value;
    int ch = pwm_channel[pin];
    if (ch < 0) {
        if (pwm_used == 0) sdk_ledc_timer(0, 8, 1);
        for (ch = 0; ch < 6 && pin_of_channel[ch] >= 0 && pin_of_channel[ch] != pin; ch++) { }
        if (ch == 6) return;                        /* ya se usaron los 6 canales */
        pin_of_channel[ch] = pin;
        pwm_channel[pin] = ch;
        pwm_used++;
        sdk_ledc_channel(ch, 0, pin, duty);
    } else {
        sdk_ledc_duty(ch, duty);
    }
}

extern "C" int analogRead(uint8_t pin) { (void)pin; return 0; }

/* ------------------------------------------------------------------ tiempo */
extern "C" unsigned long millis(void) { return sdk_millis(); }
extern "C" unsigned long micros(void) { return (unsigned long)sdk_micros(); }
extern "C" void delay(unsigned long ms) { sdk_delay_ms(ms); }
extern "C" void delayMicroseconds(unsigned int us) { sdk_delay_us(us); }
extern "C" void yield(void) { }

/* ------------------------------------------------------------------ interrupciones */
static void isr_trampoline(void *arg) { ((void (*)(void))arg)(); }

extern "C" void attachInterrupt(uint8_t pin, void (*isr)(void), int mode)
{
    int t = mode == RISING ? GPIO_INTR_POSEDGE : mode == FALLING ? GPIO_INTR_NEGEDGE :
            mode == ONLOW ? GPIO_INTR_LOW_LEVEL : mode == ONHIGH ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_ANYEDGE;
    gpio_pin_isr(pin, t, isr_trampoline, (void *)isr);
}

extern "C" void detachInterrupt(uint8_t pin) { gpio_pin_isr(pin, 0, 0, 0); }
extern "C" void interrupts(void) { intr_global_enable(); }
extern "C" void noInterrupts(void) { intr_global_disable(); }

/* ------------------------------------------------------------------ matematicas y bits */
long random(long howbig) { return howbig <= 0 ? 0 : rand() % howbig; }
long random(long lo, long hi) { return lo >= hi ? lo : lo + random(hi - lo); }
extern "C" void randomSeed(unsigned long seed) { srand(seed); }
extern "C" long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

extern "C" void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    for (int i = 0; i < 8; i++) {
        digitalWrite(dataPin, bitOrder == LSBFIRST ? (val >> i) & 1 : (val >> (7 - i)) & 1);
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

extern "C" uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
    uint8_t v = 0;
    for (int i = 0; i < 8; i++) {
        digitalWrite(clockPin, HIGH);
        if (bitOrder == LSBFIRST) v |= digitalRead(dataPin) << i;
        else v |= digitalRead(dataPin) << (7 - i);
        digitalWrite(clockPin, LOW);
    }
    return v;
}

/* ------------------------------------------------------------------ String */
void String::grow(unsigned int n)
{
    if (n + 1 <= cap) return;
    unsigned int c = cap ? cap : 16;
    while (c < n + 1) c *= 2;
    char *nb = (char *)malloc(c);
    if (!nb) return;
    memcpy(nb, buf, len + 1);
    if (cap) free(buf);
    buf = nb;
    cap = c;
}

void String::set(const char *s)
{
    unsigned int n = strlen(s);
    grow(n);
    if (n + 1 > cap) return;
    memmove(buf, s, n + 1);
    len = n;
}

String::String(const char *s) : buf((char *)""), len(0), cap(0) { set(s ? s : ""); }
String::String(const String &s) : buf((char *)""), len(0), cap(0) { set(s.buf); }
String::String(char c) : buf((char *)""), len(0), cap(0) { char b[2] = {c, 0}; set(b); }

static void num_to_str(char *out, unsigned long v, int base)
{
    char tmp[34];
    int i = 0;
    if (base < 2 || base > 16) base = 10;
    do { tmp[i++] = "0123456789ABCDEF"[v % base]; v /= base; } while (v);
    while (i) *out++ = tmp[--i];
    *out = 0;
}

String::String(int v, unsigned char base) : String((long)v, base) {}
String::String(unsigned int v, unsigned char base) : String((unsigned long)v, base) {}
String::String(long v, unsigned char base) : buf((char *)""), len(0), cap(0)
{
    char b[36];
    if (base == 10 && v < 0) { b[0] = '-'; num_to_str(b + 1, (unsigned long)(-v), 10); }
    else num_to_str(b, (unsigned long)v, base);
    set(b);
}
String::String(unsigned long v, unsigned char base) : buf((char *)""), len(0), cap(0)
{
    char b[36];
    num_to_str(b, v, base);
    set(b);
}
String::String(double v, unsigned char decimals) : buf((char *)""), len(0), cap(0)
{
    char b[48];
    snprintf(b, sizeof b, "%.*f", (int)decimals, v);
    set(b);
}
String::~String() { if (cap) free(buf); }
String &String::operator=(const String &s) { if (this != &s) set(s.buf); return *this; }
String &String::operator=(const char *s) { set(s ? s : ""); return *this; }

bool String::concat(const char *s)
{
    unsigned int n = strlen(s);
    grow(len + n);
    if (len + n + 1 > cap) return false;
    memcpy(buf + len, s, n + 1);
    len += n;
    return true;
}

bool String::equalsIgnoreCase(const String &s) const
{
    if (len != s.len) return false;
    for (unsigned int i = 0; i < len; i++) if (tolower(buf[i]) != tolower(s.buf[i])) return false;
    return true;
}

int String::indexOf(char c, unsigned int from) const
{
    for (unsigned int i = from; i < len; i++) if (buf[i] == c) return i;
    return -1;
}

int String::indexOf(const String &s, unsigned int from) const
{
    if (from > len) return -1;
    const char *p = strstr(buf + from, s.buf);
    return p ? (int)(p - buf) : -1;
}

int String::lastIndexOf(char c) const
{
    for (int i = (int)len - 1; i >= 0; i--) if (buf[i] == c) return i;
    return -1;
}

String String::substring(unsigned int from, unsigned int to) const
{
    if (to > len) to = len;
    if (from > to) { unsigned int t = from; from = to; to = t; }
    String r;
    r.grow(to - from);
    if (r.cap) {
        memcpy(r.buf, buf + from, to - from);
        r.buf[to - from] = 0;
        r.len = to - from;
    }
    return r;
}

void String::remove(unsigned int index, unsigned int count)
{
    if (index >= len) return;
    if (count > len - index) count = len - index;
    memmove(buf + index, buf + index + count, len - index - count + 1);
    len -= count;
}

void String::replace(const String &find, const String &repl)
{
    if (!find.len) return;
    String out;
    unsigned int i = 0;
    while (i < len) {
        if (strncmp(buf + i, find.buf, find.len) == 0) { out += repl; i += find.len; }
        else { out += buf[i]; i++; }
    }
    *this = out;
}

void String::toUpperCase() { for (unsigned int i = 0; i < len; i++) buf[i] = toupper(buf[i]); }
void String::toLowerCase() { for (unsigned int i = 0; i < len; i++) buf[i] = tolower(buf[i]); }
void String::trim()
{
    unsigned int a = 0, b = len;
    while (a < b && isspace(buf[a])) a++;
    while (b > a && isspace(buf[b - 1])) b--;
    *this = substring(a, b);
}

String operator+(const String &a, const String &b) { String r(a); r += b; return r; }
String operator+(const String &a, const char *b) { String r(a); r += b; return r; }
String operator+(const char *a, const String &b) { String r(a); r += b; return r; }
String operator+(const String &a, char b) { String r(a); r += b; return r; }
String operator+(const String &a, int b) { String r(a); r += b; return r; }
String operator+(const String &a, unsigned int b) { String r(a); r += b; return r; }
String operator+(const String &a, long b) { String r(a); r += b; return r; }
String operator+(const String &a, unsigned long b) { String r(a); r += b; return r; }
String operator+(const String &a, double b) { String r(a); r += b; return r; }

/* ------------------------------------------------------------------ Serial */
HardwareSerial Serial;

int HardwareSerial::available() { return uart_available(0) + (peeked >= 0 ? 1 : 0); }

int HardwareSerial::read()
{
    if (peeked >= 0) { int c = peeked; peeked = -1; return c; }
    return uart_getc(0);
}

int HardwareSerial::peek()
{
    if (peeked < 0) peeked = uart_getc(0);
    return peeked;
}

size_t HardwareSerial::write(uint8_t c) { uart_putc(0, (char)c); return 1; }
size_t HardwareSerial::write(const char *s) { size_t n = strlen(s); uart_write_n(0, s, (int)n, 0); return n; }
size_t HardwareSerial::write(const uint8_t *b, size_t n) { uart_write_n(0, (const char *)b, (int)n, 0); return n; }

size_t HardwareSerial::print(long v, int base)
{
    char b[36];
    if (base == DEC && v < 0) { b[0] = '-'; num_to_str(b + 1, (unsigned long)(-v), 10); }
    else num_to_str(b, (unsigned long)v, base);
    return write(b);
}

size_t HardwareSerial::print(unsigned long v, int base)
{
    char b[36];
    num_to_str(b, v, base);
    return write(b);
}

size_t HardwareSerial::print(long long v, int base)
{
    char b[24];
    if (base == HEX) snprintf(b, sizeof b, "%llX", (unsigned long long)v);
    else snprintf(b, sizeof b, "%lld", v);
    return write(b);
}

size_t HardwareSerial::print(unsigned long long v, int base)
{
    char b[24];
    snprintf(b, sizeof b, base == HEX ? "%llX" : "%llu", v);
    return write(b);
}

size_t HardwareSerial::print(double v, int digits)
{
    char b[48];
    snprintf(b, sizeof b, "%.*f", digits, v);
    return write(b);
}

size_t HardwareSerial::printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vprintf(fmt, ap);          /* \n -> \r\n, directo a la UART0 */
    va_end(ap);
    return n > 0 ? (size_t)n : 0;
}

String HardwareSerial::readStringUntil(char terminator)
{
    String s;
    unsigned long t0 = millis();
    while (millis() - t0 < timeout) {
        int c = read();
        if (c < 0) continue;
        if (c == terminator) break;
        s += (char)c;
        t0 = millis();
    }
    return s;
}

String HardwareSerial::readString() { return readStringUntil(0); }

long HardwareSerial::parseInt()
{
    unsigned long t0 = millis();
    int c;
    for (;;) {                                          /* salta lo que no sea numero */
        c = peek();
        if (c == '-' || (c >= '0' && c <= '9')) break;
        if (c >= 0) read();
        if (millis() - t0 >= timeout) return 0;
    }
    long v = 0;
    int neg = 0;
    if (c == '-') { neg = 1; read(); }
    t0 = millis();
    while (millis() - t0 < timeout) {
        c = peek();
        if (c < 0) continue;
        if (c < '0' || c > '9') break;
        v = v * 10 + (read() - '0');
        t0 = millis();
    }
    return neg ? -v : v;
}

/* ------------------------------------------------------------------ Wire (I2C) */
TwoWire Wire;

bool TwoWire::begin(int sda, int scl, uint32_t freq)
{
    (void)freq;
    i2c_init(sda, scl);
    return true;
}

void TwoWire::beginTransmission(uint8_t address) { addr = address; tx_len = 0; pending = false; }

size_t TwoWire::write(uint8_t b)
{
    if (tx_len >= (int)sizeof tx_buf) return 0;
    tx_buf[tx_len++] = b;
    return 1;
}

size_t TwoWire::write(const uint8_t *d, size_t n)
{
    size_t k = 0;
    while (k < n && write(d[k])) k++;
    return k;
}

uint8_t TwoWire::endTransmission(bool sendStop)
{
    if (!sendStop) { pending = true; return 0; }
    pending = false;
    return i2c_write(addr, tx_buf, tx_len) == 0 ? 0 : 2;
}

size_t TwoWire::requestFrom(uint8_t address, size_t n, bool sendStop)
{
    (void)sendStop;
    if (n > sizeof rx_buf) n = sizeof rx_buf;
    int r;
    if (pending && address == addr) r = i2c_write_read(address, tx_buf, tx_len, rx_buf, n);
    else r = i2c_read(address, rx_buf, n);
    pending = false;
    rx_pos = 0;
    rx_len = r == 0 ? (int)n : 0;
    return rx_len;
}

/* ------------------------------------------------------------------ SPI */
SPIClass SPI;

void SPIClass::begin(int sck, int miso, int mosi, int ss)
{
    (void)sck; (void)miso; (void)mosi; (void)ss;     /* el modelo usa los pines del IO MUX: 6, 2, 7 y 10 */
    spi_init(2);
}

uint8_t SPIClass::transfer(uint8_t data)
{
    uint8_t r = 0;
    spi_transfer(&data, &r, 1);
    return r;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    uint8_t t[2] = {(uint8_t)(data >> 8), (uint8_t)data}, r[2] = {0, 0};
    spi_transfer(t, r, 2);
    return (uint16_t)(r[0] << 8 | r[1]);
}

void SPIClass::transfer(void *buf, size_t n) { spi_transfer((uint8_t *)buf, (uint8_t *)buf, (int)n); }

/* ------------------------------------------------------------------ C++ sin biblioteca estandar */
void *operator new(size_t n) { return malloc(n); }
void *operator new[](size_t n) { return malloc(n); }
void operator delete(void *p) noexcept { free(p); }
void operator delete[](void *p) noexcept { free(p); }
void operator delete(void *p, size_t) noexcept { free(p); }
void operator delete[](void *p, size_t) noexcept { free(p); }
extern "C" void __cxa_pure_virtual(void) { abort(); }
extern "C" int __cxa_atexit(void (*f)(void *), void *a, void *d) { (void)f; (void)a; (void)d; return 0; }
extern "C" { void *__dso_handle = 0; }

/* ------------------------------------------------------------------ programa principal */
extern "C" int main(void)
{
    setup();
    for (;;) loop();
}
