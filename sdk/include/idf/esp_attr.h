#ifndef ESP_ATTR_H
#define ESP_ATTR_H
#define IRAM_ATTR
#define DRAM_ATTR
#define RTC_DATA_ATTR
#define RTC_NOINIT_ATTR
#define WORD_ALIGNED_ATTR __attribute__((aligned(4)))
#define NOINLINE_ATTR __attribute__((noinline))
#endif
