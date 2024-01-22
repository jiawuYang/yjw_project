#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RET_MALLOC_ERROR = -6,
    RET_NO_SPACE = -5,
    RET_NOT_EXISTS = -4,
    RET_TIMEOUT = -3,
    RET_INVALID_PARAM = -2,
    RET_ERROR = -1,
    RET_OK = 0,
} ErrorCode;

#ifdef __cplusplus
}
#endif