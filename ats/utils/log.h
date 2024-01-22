#pragma once

#include <stdio.h>

typedef enum {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_DISABLE
} LogLevel;

#ifndef CUR_LOG_LEVEL
#define CUR_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#define LOG(ch, level, fmt, args...)                                            \
    do {                                                                        \
        if (level >= CUR_LOG_LEVEL) {                                           \
            printf("%c/%s[%d]: " fmt "\n", ch, __FUNCTION__, __LINE__, ##args); \
        }                                                                       \
    } while (0)

#define LOG_F(fmt, args...) LOG('F', LOG_LEVEL_FATAL, fmt, ##args)
#define LOG_E(fmt, args...) LOG('E', LOG_LEVEL_ERROR, fmt, ##args)
#define LOG_W(fmt, args...) LOG('W', LOG_LEVEL_WARN, fmt, ##args)
#define LOG_I(fmt, args...) LOG('I', LOG_LEVEL_INFO, fmt, ##args)
#define LOG_D(fmt, args...) LOG('D', LOG_LEVEL_DEBUG, fmt, ##args)
#define LOG_T(fmt, args...) LOG('T', LOG_LEVEL_TRACE, fmt, ##args)