#ifndef SDK_ASSERT_H
#define SDK_ASSERT_H
#ifdef __cplusplus
extern "C" {
#endif
void __assert_func(const char *file, int line, const char *fn, const char *expr) __attribute__((noreturn));
#ifdef __cplusplus
}
#endif
#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#define assert(e) ((e) ? (void)0 : __assert_func(__FILE__, __LINE__, __func__, #e))
#endif
#endif
