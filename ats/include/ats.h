/**
 * Copyright (c) 2023 Tonly Technology Co., Ltd.,
 * All rights reserved.
 */

#pragma once

#include "dlist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*SendToPc)(const char *fmt, ...);

typedef struct {
    /* cmd string */
    const char *str;
    /* callback of cmd */
    int (*cb)(int argc, char *argv[], SendToPc toPc);
} AtCmd;

typedef struct {
    AtCmd cmd;
    /* list handle */
    list_head_t list;
} AtCmdNode;

typedef struct {
    AtCmd *cmds;
    int len;
    /* list handle */
    list_head_t list;
} ModuleNode;

/*
 * description: 指令注册
 * cmd: 需要注册的指令
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdRegister(AtCmd cmd);

/*
 * description: 指令反注册
 * cmd: 需要反注册的指令
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdUnregister(AtCmd cmd);

/*
 * description: 指令执行
 * str: 指向待解析字符串
 * len: 待解析字符串长度
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdExec(const char *str, int len, SendToPc toPc);

int AtCmdInit(void);

int AtCmdDeInit(void);

#ifdef __cplusplus
}
#endif