#pragma once

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * return
 * P: print    打印提示
 * R: resource 资源释放
 * V: void     无返回值
 */

/* condition 不为真返回 ret */
#define CHECK_RETURN(condition, ret)     \
    do {                                 \
        if (!(condition)) {              \
            LOG_E("condition not met."); \
            return (ret);                \
        }                                \
    } while (0)

/* condition 不为真返回 ret，打印提示 */
#define CHECK_P_RETURN(condition, ret, format, ...) \
    do {                                            \
        if (!(condition)) {                         \
            LOG_E(format, ##__VA_ARGS__);           \
            return (ret);                           \
        }                                           \
    } while (0)

/* condition 不为真返回 ret，并释放资源(关闭文件、free 内存等) */
#define CHECK_R_RETURN(condition, ret, release) \
    do {                                        \
        if (!(condition)) {                     \
            LOG_E("condition not met.");        \
            release;                            \
            return (ret);                       \
        }                                       \
    } while (0)

/* condition 不为真返回，针对无返回值函数 */
#define CHECK_V_RETURN(condition)        \
    do {                                 \
        if (!(condition)) {              \
            LOG_E("condition not met."); \
            return;                      \
        }                                \
    } while (0)

/* condition 不为真返回 ret，打印提示参数，并释放资源(关闭文件、free 内存等) */
#define CHECK_PR_RETURN(condition, ret, release, format, ...) \
    do {                                                      \
        if (!(condition)) {                                   \
            LOG_E(format, ##__VA_ARGS__);                     \
            release;                                          \
            return (ret);                                     \
        }                                                     \
    } while (0)

/* condition 不为真返回，打印提示参数，针对无返回值函数 */
#define CHECK_PV_RETURN(condition, format, ...) \
    do {                                        \
        if (!(condition)) {                     \
            LOG_E(format, ##__VA_ARGS__);       \
            return;                             \
        }                                       \
    } while (0)

/* condition 不为真返回，释放资源(关闭文件、free 内存等)，针对无返回值函数 */
#define CHECK_RV_RETURN(condition, release) \
    do {                                    \
        if (!(condition)) {                 \
            LOG_E("condition not met.");    \
            release;                        \
            return;                         \
        }                                   \
    } while (0)

/* 函数指针参数ptr判空返回，针对无返回值函数 */
#define CHECK_PRV_RETURN(condition, release, format, ...) \
    do {                                                  \
        if (!(condition)) {                               \
            LOG_E(format, ##__VA_ARGS__);                 \
            release;                                      \
            return;                                       \
        }                                                 \
    } while (0)

#ifdef __cplusplus
}
#endif
