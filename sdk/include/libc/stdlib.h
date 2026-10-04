#ifndef SDK_STDLIB_H
#define SDK_STDLIB_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RAND_MAX 0x7FFFFFFF
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
void *malloc(size_t n);
void *calloc(size_t n, size_t s);
void *realloc(void *p, size_t n);
void free(void *p);
int abs(int x);
long labs(long x);
int atoi(const char *s);
long atol(const char *s);
double atof(const char *s);
long strtol(const char *s, char **end, int base);
unsigned long strtoul(const char *s, char **end, int base);
int rand(void);
void srand(unsigned s);
void abort(void) __attribute__((noreturn));
void exit(int code) __attribute__((noreturn));
int atexit(void (*f)(void));
#ifdef __cplusplus
}
#endif
#endif
