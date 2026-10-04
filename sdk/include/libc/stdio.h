/* stdio.h minimo del SDK del modelo (la salida va a la UART0: GPIO21 -> terminal de la placa) */
#ifndef SDK_STDIO_H
#define SDK_STDIO_H
#include <stddef.h>
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif
#define EOF (-1)
typedef struct sdk_file FILE;
#define stdin  ((FILE *)0)
#define stdout ((FILE *)1)
#define stderr ((FILE *)2)
int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int sprintf(char *s, const char *fmt, ...);
int snprintf(char *s, size_t n, const char *fmt, ...);
int vsnprintf(char *s, size_t n, const char *fmt, va_list ap);
int putchar(int c);
int puts(const char *s);
int getchar(void);                       /* espera una tecla del teclado de la placa */
int fflush(FILE *f);
int fputs(const char *s, FILE *f);
int fputc(int c, FILE *f);
int fprintf(FILE *f, const char *fmt, ...);
int vfprintf(FILE *f, const char *fmt, va_list ap);
size_t fwrite(const void *p, size_t size, size_t n, FILE *f);
#define putc(c, f) fputc(c, f)
#ifdef __cplusplus
}
#endif
#endif
