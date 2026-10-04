/*
 * libc_min.c - Un "libc" minimo para el modelo: memoria, cadenas, malloc, printf y compania.
 * Se usa en lugar de la libc del compilador para no depender de como venga instalada
 * (y porque es mucho mas corta: en Logisim cada instruccion cuenta).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "esp32c3.h"

/* ------------------------------------------------------------------ memoria y cadenas */
void *memcpy(void *d, const void *s, size_t n)
{
    uint8_t *dp = d;
    const uint8_t *sp = s;
    if ((((uintptr_t)dp | (uintptr_t)sp) & 3) == 0) {
        while (n >= 4) { *(uint32_t *)dp = *(const uint32_t *)sp; dp += 4; sp += 4; n -= 4; }
    }
    while (n--) *dp++ = *sp++;
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    uint8_t *dp = d;
    const uint8_t *sp = s;
    if (dp < sp) { while (n--) *dp++ = *sp++; }
    else { dp += n; sp += n; while (n--) *--dp = *--sp; }
    return d;
}

void *memset(void *d, int c, size_t n)
{
    uint8_t *dp = d;
    while (n && ((uintptr_t)dp & 3)) { *dp++ = (uint8_t)c; n--; }
    uint32_t w = (uint8_t)c * 0x01010101u;
    while (n >= 4) { *(uint32_t *)dp = w; dp += 4; n -= 4; }
    while (n--) *dp++ = (uint8_t)c;
    return d;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *x = a, *y = b;
    for (; n; n--, x++, y++) if (*x != *y) return *x - *y;
    return 0;
}

size_t strlen(const char *s) { const char *p = s; while (*p) p++; return p - s; }
size_t strnlen(const char *s, size_t m) { size_t n = 0; while (n < m && s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (uint8_t)*a - (uint8_t)*b; }
int strncmp(const char *a, const char *b, size_t n)
{
    for (; n; n--, a++, b++) { if (*a != *b || !*a) return (uint8_t)*a - (uint8_t)*b; }
    return 0;
}
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)) { } return r; }
char *strncpy(char *d, const char *s, size_t n)
{
    char *r = d;
    while (n && (*d = *s)) { d++; s++; n--; }
    while (n--) *d++ = 0;
    return r;
}
char *strcat(char *d, const char *s) { strcpy(d + strlen(d), s); return d; }
char *strncat(char *d, const char *s, size_t n)
{
    char *e = d + strlen(d);
    while (n-- && *s) *e++ = *s++;
    *e = 0;
    return d;
}
char *strchr(const char *s, int c) { for (;; s++) { if (*s == (char)c) return (char *)s; if (!*s) return 0; } }
char *strrchr(const char *s, int c) { const char *r = 0; for (;; s++) { if (*s == (char)c) r = s; if (!*s) return (char *)r; } }
char *strstr(const char *h, const char *n)
{
    size_t k = strlen(n);
    if (!k) return (char *)h;
    for (; *h; h++) if (!strncmp(h, n, k)) return (char *)h;
    return 0;
}

int abs(int x) { return x < 0 ? -x : x; }
long labs(long x) { return x < 0 ? -x : x; }
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isspace(int c) { return c == ' ' || (c >= 9 && c <= 13); }
int isalpha(int c) { return (c | 32) >= 'a' && (c | 32) <= 'z'; }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }
int isprint(int c) { return c >= 32 && c < 127; }
int isxdigit(int c) { return isdigit(c) || ((c | 32) >= 'a' && (c | 32) <= 'f'); }
int toupper(int c) { return islower(c) ? c - 32 : c; }
int tolower(int c) { return isupper(c) ? c + 32 : c; }

long strtol(const char *s, char **end, int base)
{
    while (isspace(*s)) s++;
    int neg = 0;
    if (*s == '-' || *s == '+') neg = (*s++ == '-');
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] | 32) == 'x') { s += 2; base = 16; }
    else if (base == 0) base = (s[0] == '0') ? 8 : 10;
    long v = 0;
    for (;; s++) {
        int d = isdigit(*s) ? *s - '0' : isalpha(*s) ? (*s | 32) - 'a' + 10 : 99;
        if (d >= base) break;
        v = v * base + d;
    }
    if (end) *end = (char *)s;
    return neg ? -v : v;
}
unsigned long strtoul(const char *s, char **end, int base) { return (unsigned long)strtol(s, end, base); }
int atoi(const char *s) { return (int)strtol(s, 0, 10); }
long atol(const char *s) { return strtol(s, 0, 10); }

double atof(const char *s)
{
    while (isspace(*s)) s++;
    int neg = 0;
    if (*s == '-' || *s == '+') neg = (*s++ == '-');
    double v = 0, f = 0.1;
    while (isdigit(*s)) v = v * 10 + (*s++ - '0');
    if (*s == '.') { s++; while (isdigit(*s)) { v += (*s++ - '0') * f; f *= 0.1; } }
    return neg ? -v : v;
}

static uint32_t rand_state = 1;
void srand(unsigned s) { rand_state = s ? s : 1; }
int rand(void)
{
    rand_state = rand_state * 1103515245u + 12345u;
    return (rand_state >> 1) & 0x7FFFFFFF;
}

/* ------------------------------------------------------------------ malloc / free
 * Lista de bloques libres (primer ajuste) entre el final de .bss y la zona de la pila. */
extern char _heap_start[], _heap_end[];
typedef struct blk { size_t size; struct blk *next; } blk_t;     /* size incluye la cabecera */
static blk_t *free_list;
static char *heap_top;

void *malloc(size_t n)
{
    if (!n) return 0;
    n = (n + sizeof(blk_t) + 7) & ~7u;
    blk_t **pp = &free_list;
    for (blk_t *b = free_list; b; pp = &b->next, b = b->next) {
        if (b->size >= n) {
            if (b->size >= n + 32) {                    /* parte el bloque */
                blk_t *r = (blk_t *)((char *)b + n);
                r->size = b->size - n;
                r->next = b->next;
                *pp = r;
                b->size = n;
            } else {
                *pp = b->next;
            }
            return b + 1;
        }
    }
    if (!heap_top) heap_top = (char *)(((uintptr_t)_heap_start + 7) & ~7u);
    if (heap_top + n > _heap_end) return 0;
    blk_t *b = (blk_t *)heap_top;
    heap_top += n;
    b->size = n;
    return b + 1;
}

void free(void *p)
{
    if (!p) return;
    blk_t *b = (blk_t *)p - 1;
    b->next = free_list;
    free_list = b;
}

void *calloc(size_t n, size_t s) { void *p = malloc(n * s); if (p) memset(p, 0, n * s); return p; }

void *realloc(void *p, size_t n)
{
    if (!p) return malloc(n);
    blk_t *b = (blk_t *)p - 1;
    if (b->size - sizeof(blk_t) >= n) return p;
    void *q = malloc(n);
    if (q) { memcpy(q, p, b->size - sizeof(blk_t)); free(p); }
    return q;
}

size_t heap_free_bytes(void) { return heap_top ? (size_t)(_heap_end - heap_top) : (size_t)(_heap_end - _heap_start); }

/* ------------------------------------------------------------------ consola (UART0) */
/* escritura directa en la FIFO de la UART (sin capas: en Logisim cada instruccion cuenta) */
static inline void tx0(char c)
{
    while (UART_TXFIFO_CNT(REG_READ(UART_STATUS_REG(0))) >= 120) { }
    REG_WRITE(UART_FIFO_REG(0), (uint8_t)c);
}

/* Escribe n bytes en la FIFO de la UART 'u' (crlf = 1: cambia \n por \r\n, como una terminal).
 * Pregunta UNA vez cuanto espacio queda en la FIFO (128 bytes) y escribe esa tanda sin volver a preguntar:
 * asi cada caracter cuesta unas 8 instrucciones y la CPU termina antes que la UART, que sigue enviando sola. */
void uart_write_n(int u, const char *s, int n, int crlf)
{
    volatile uint32_t *fifo = (volatile uint32_t *)UART_FIFO_REG(u);
    while (n > 0) {
        int room = 126 - (int)UART_TXFIFO_CNT(REG_READ(UART_STATUS_REG(u)));
        if (crlf) {
            while (room >= 2 && n > 0) {
                char c = *s++;
                n--;
                if (c == '\n') { *fifo = '\r'; room--; }
                *fifo = (uint8_t)c;
                room--;
            }
        } else {
            while (room > 0 && n > 0) { *fifo = (uint8_t)*s++; n--; room--; }
        }
    }
}

int putchar(int c)
{
    if (c == '\n') tx0('\r');
    tx0((char)c);
    return (uint8_t)c;
}

int puts(const char *s)
{
    uart_write_n(0, s, (int)strlen(s), 1);
    uart_write_n(0, "\n", 1, 1);
    return 1;
}

/* ------------------------------------------------------------------ printf
 * %d %i %u %x %X %o %c %s %p %f %% con ancho, precision, '-', '0', '+', ' ' y tamanos h hh l ll z
 * La salida va por "tandas" (out recibe varios caracteres a la vez): el texto fijo del formato se manda
 * entero de una vez, no letra por letra. */
typedef void (*out_fn)(const char *s, int n, void *ctx);

static int pad(out_fn out, void *ctx, char c, int n)
{
    for (int i = 0; i < n; i++) out(&c, 1, ctx);
    return n > 0 ? n : 0;
}

static int out_str(out_fn out, void *ctx, const char *s, int len, int width, int left)
{
    int n = 0;
    if (!left) n += pad(out, ctx, ' ', width - len);
    out(s, len, ctx);
    n += len;
    if (left) n += pad(out, ctx, ' ', width - len);
    return n;
}

static int fmt_core(out_fn out, void *ctx, const char *f, va_list ap)
{
    int count = 0;
    char buf[40];
    for (; *f; f++) {
        if (*f != '%') {
            const char *q = f + 1;
            while (*q && *q != '%') q++;
            out(f, (int)(q - f), ctx);
            count += (int)(q - f);
            f = q - 1;
            continue;
        }
        f++;
        int left = 0, zero = 0, plus = 0, space = 0, width = 0, prec = -1, lng = 0;
        for (;; f++) {
            if (*f == '-') left = 1;
            else if (*f == '0') zero = 1;
            else if (*f == '+') plus = 1;
            else if (*f == ' ') space = 1;
            else if (*f == '#') { }
            else break;
        }
        if (*f == '*') { width = va_arg(ap, int); f++; }
        else while (isdigit(*f)) width = width * 10 + (*f++ - '0');
        if (*f == '.') {
            f++;
            prec = 0;
            if (*f == '*') { prec = va_arg(ap, int); f++; }
            else while (isdigit(*f)) prec = prec * 10 + (*f++ - '0');
        }
        while (*f == 'l' || *f == 'h' || *f == 'z' || *f == 'j' || *f == 't') { if (*f == 'l') lng++; f++; }
        char c = *f;
        if (!c) break;
        if (c == 's') {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            int len = prec >= 0 ? (int)strnlen(s, prec) : (int)strlen(s);
            count += out_str(out, ctx, s, len, width, left);
            continue;
        }
        if (c == 'c') { buf[0] = (char)va_arg(ap, int); count += out_str(out, ctx, buf, 1, width, left); continue; }
        if (c == '%') { out("%", 1, ctx); count++; continue; }
        int neg = 0, len = 0;
        char *p = buf + sizeof buf;
        if (c == 'f' || c == 'F' || c == 'g' || c == 'e' || c == 'G' || c == 'E') {
            double v = va_arg(ap, double);
            if (prec < 0) prec = 6;
            if (prec > 9) prec = 9;
            if (v != v) { count += out_str(out, ctx, "nan", 3, width, left); continue; }
            if (v < 0) { neg = 1; v = -v; }
            uint32_t scale = 1;
            for (int i = 0; i < prec; i++) scale *= 10;
            double r = v * scale + 0.5;
            if (r >= 1.8e19) { count += out_str(out, ctx, neg ? "-inf" : "inf", neg ? 4 : 3, width, left); continue; }
            uint64_t t = (uint64_t)r;
            uint64_t ip = t / scale;
            uint32_t fp = (uint32_t)(t % scale);
            for (int i = 0; i < prec; i++) { *--p = '0' + fp % 10; fp /= 10; }
            if (prec) *--p = '.';
            do { *--p = '0' + (int)(ip % 10); ip /= 10; } while (ip);
        } else {
            uint64_t v;
            int base = 10, upper = 0;
            if (c == 'd' || c == 'i') {
                int64_t s = lng >= 2 ? va_arg(ap, int64_t) : (int64_t)va_arg(ap, long);
                if (s < 0) { neg = 1; v = (uint64_t)(-s); } else v = (uint64_t)s;
            } else {
                v = lng >= 2 ? va_arg(ap, uint64_t) : (uint64_t)va_arg(ap, unsigned long);
                if (c == 'x' || c == 'p') base = 16;
                else if (c == 'X') { base = 16; upper = 1; }
                else if (c == 'o') base = 8;
                if (c == 'p') { zero = 1; width = 8; out("0x", 2, ctx); count += 2; }
            }
            const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";
            if (v <= 0xFFFFFFFFu) {
                uint32_t w = (uint32_t)v;
                do { *--p = dig[w % base]; w /= base; } while (w);
            } else {
                do { *--p = dig[v % base]; v /= base; } while (v);
            }
            while (prec > 0 && (buf + sizeof buf - p) < prec) *--p = '0';
        }
        len = buf + sizeof buf - p;
        char sign = neg ? '-' : plus ? '+' : space ? ' ' : 0;
        if (zero && !left) {
            if (sign) { out(&sign, 1, ctx); count++; width--; }
            count += pad(out, ctx, '0', width - len);
            count += out_str(out, ctx, p, len, 0, 0);
        } else {
            if (sign) { *--p = sign; len++; }
            count += out_str(out, ctx, p, len, width, left);
        }
    }
    return count;
}

static void out_uart(const char *s, int n, void *ctx)
{
    (void)ctx;
    uart_write_n(0, s, n, 1);
}

int vprintf(const char *f, va_list ap) { return fmt_core(out_uart, 0, f, ap); }

int printf(const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = vprintf(f, ap);
    va_end(ap);
    return n;
}

typedef struct { char *p; size_t left; } sbuf_t;
static void out_buf(const char *s, int n, void *ctx)
{
    sbuf_t *b = ctx;
    while (n-- > 0 && b->left > 1) { *b->p++ = *s++; b->left--; }
}

int vsnprintf(char *s, size_t n, const char *f, va_list ap)
{
    sbuf_t b = {s, n};
    int r = fmt_core(out_buf, &b, f, ap);
    if (n) *b.p = 0;
    return r;
}

int snprintf(char *s, size_t n, const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(s, n, f, ap);
    va_end(ap);
    return r;
}

int sprintf(char *s, const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(s, 0x7FFFFFFF, f, ap);
    va_end(ap);
    return r;
}

/* compatibilidad con los ejemplos de la primera version */
int printf_(const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = vprintf(f, ap);
    va_end(ap);
    return n;
}
int vprintf_(const char *f, va_list ap) { return vprintf(f, ap); }
void putchar_(char c) { putchar(c); }
void puts_(const char *s) { uart_write_n(0, s, (int)strlen(s), 1); }

/* ------------------------------------------------------------------ varios */
void abort(void)
{
    printf("\nabort()\n");
    __asm__ volatile("ebreak");
    for (;;) { }
}

void __assert_func(const char *file, int line, const char *fn, const char *expr)
{
    printf("\nassert fallo: %s (%s:%d, %s)\n", expr, file, line, fn ? fn : "");
    abort();
}

int atexit(void (*f)(void)) { (void)f; return 0; }
void exit(int code)
{
    printf("\nexit(%d)\n", code);
    __asm__ volatile("ebreak");
    for (;;) { }
}

/* ------------------------------------------------------------------ mas stdio */
int sdk_uart_getc(int u);
int getchar(void)
{
    int c;
    while ((c = sdk_uart_getc(0)) < 0) { }
    return c;
}
typedef struct sdk_file FILE;
int fflush(FILE *f) { (void)f; return 0; }
int fputc(int c, FILE *f) { (void)f; return putchar(c); }
int fputs(const char *s, FILE *f) { (void)f; uart_write_n(0, s, (int)strlen(s), 1); return 1; }
size_t fwrite(const void *p, size_t sz, size_t n, FILE *f) { (void)f; uart_write_n(0, p, (int)(sz * n), 1); return n; }
int fprintf(FILE *f, const char *fmt, ...)
{
    (void)f;
    va_list ap;
    va_start(ap, fmt);
    int n = vprintf(fmt, ap);
    va_end(ap);
    return n;
}
int vfprintf(FILE *f, const char *fmt, va_list ap) { (void)f; return vprintf(fmt, ap); }

/* ------------------------------------------------------------------ matematicas (por software) */
double fabs(double x) { return x < 0 ? -x : x; }
float fabsf(float x) { return x < 0 ? -x : x; }
double floor(double x)
{
    if (x >= 9.0e18 || x <= -9.0e18) return x;
    int64_t i = (int64_t)x;
    return (double)(x < 0 && (double)i != x ? i - 1 : i);
}
double ceil(double x) { double f = floor(x); return f == x ? f : f + 1; }
double round(double x) { return x < 0 ? -floor(-x + 0.5) : floor(x + 0.5); }
double fmod(double x, double y) { if (y == 0) return 0; double q = x / y; q = q < 0 ? ceil(q) : floor(q); return x - q * y; }
double sqrt(double x)
{
    if (x <= 0) return 0;
    double r = x > 1 ? x / 2 : 1;
    for (int i = 0; i < 30; i++) { double n = 0.5 * (r + x / r); if (n == r) break; r = n; }
    return r;
}
float sqrtf(float x) { return (float)sqrt(x); }
double exp(double x)
{
    int k = (int)(x / 0.6931471805599453);            /* x = k ln2 + r */
    double r = x - k * 0.6931471805599453, t = 1, s = 1;
    for (int i = 1; i < 20; i++) { t *= r / i; s += t; }
    while (k > 0) { s *= 2; k--; }
    while (k < 0) { s /= 2; k++; }
    return s;
}
double log(double x)
{
    if (x <= 0) return -1e308;
    int k = 0;
    while (x > 2) { x /= 2; k++; }
    while (x < 1) { x *= 2; k--; }
    double y = (x - 1) / (x + 1), y2 = y * y, t = y, s = 0;   /* ln x = 2 atanh((x-1)/(x+1)) */
    for (int i = 1; i < 40; i += 2) { s += t / i; t *= y2; }
    return 2 * s + k * 0.6931471805599453;
}
double pow(double x, double y)
{
    if (y == (int)y) {
        int n = (int)y, neg = n < 0;
        double r = 1;
        if (neg) n = -n;
        while (n) { if (n & 1) r *= x; x *= x; n >>= 1; }
        return neg ? 1 / r : r;
    }
    return x <= 0 ? 0 : exp(y * log(x));
}
double sin(double x)
{
    const double pi = 3.14159265358979323846;
    x = fmod(x, 2 * pi);
    if (x > pi) x -= 2 * pi;
    if (x < -pi) x += 2 * pi;
    double t = x, s = x;
    for (int i = 1; i < 12; i++) { t *= -x * x / ((2 * i) * (2 * i + 1)); s += t; }
    return s;
}
double cos(double x) { return sin(x + 1.57079632679489661923); }
double tan(double x) { double c = cos(x); return c == 0 ? 1e308 : sin(x) / c; }
