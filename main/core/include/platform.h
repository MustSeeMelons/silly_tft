#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    PLATFORM_LOG_LEVEL_NONE = 0, // Note: Check MCU specific levels when using
    PLATFORM_LOG_LEVEL_ERROR,
    PLATFORM_LOG_LEVEL_WARN,
    PLATFORM_LOG_LEVEL_INFO,
    PLATFORM_LOG_LEVEL_DEBUG,
} platform_log_level_t;

void delay_ms(uint32_t ms);
uint64_t now_ms(void);
void platform_log(platform_log_level_t level, const char *tag, const char *fmt, ...);
void platform_set_led(bool on);

#endif