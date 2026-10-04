/* math.h minimo (sin FPU: todo se calcula por software, asi que es lento en Logisim) */
#ifndef SDK_MATH_H
#define SDK_MATH_H
#ifdef __cplusplus
extern "C" {
#endif
#define M_PI 3.14159265358979323846
#define NAN (__builtin_nan(""))
#define INFINITY (__builtin_inf())
double fabs(double x); double floor(double x); double ceil(double x); double round(double x);
double sqrt(double x); double fmod(double x, double y); double pow(double x, double y);
double sin(double x); double cos(double x); double tan(double x); double exp(double x); double log(double x);
float fabsf(float x); float sqrtf(float x);
#ifdef __cplusplus
}
#endif
#endif
