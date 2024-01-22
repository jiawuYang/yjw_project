/**
 * Copyright (c) 2023 Tonly Technology Co., Ltd.,
 * All rights reserved.
 */

#include "ats.h"
#include "assert_check.h"
#include "error_code.h"
#include "log.h"
#include <securec.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX_DILM   "="
#define PARAM_DILM    "_"
#define MAX_PARAM_CNT 32

static LIST_HEAD(g_cmdList);

LIST_HEAD(g_moduleList);

/*
 * description: 指令注册
 * cmd: 需要注册的指令
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdRegister(AtCmd cmd)
{
    CHECK_RETURN(cmd.str != NULL, RET_INVALID_PARAM);
    CHECK_RETURN(cmd.cb != NULL, RET_INVALID_PARAM);

    /* check if cmd already registered */
    struct list_head *pos = NULL;
    list_for_each(pos, &g_cmdList)
    {
        AtCmdNode *tmp = list_entry(pos, AtCmdNode, list);
        if (strcmp(cmd.str, tmp->cmd.str) == 0) {
            return RET_INVALID_PARAM;
        }
    }

    AtCmdNode *newCmd = calloc(1, sizeof(AtCmdNode));
    newCmd->cmd.str = cmd.str;
    newCmd->cmd.cb = cmd.cb;

    list_add_tail(&newCmd->list, &g_cmdList);

    return RET_OK;
}

/*
 * description: 指令反注册
 * cmd: 需要反注册的指令
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdUnregister(AtCmd cmd)
{
    CHECK_RETURN(cmd.str != NULL, RET_INVALID_PARAM);

    /* check if cmd already registered */
    struct list_head *pos = NULL;
    list_for_each(pos, &g_cmdList)
    {
        AtCmdNode *tmp = list_entry(pos, AtCmdNode, list);
        if (strcmp(cmd.str, tmp->cmd.str) == 0) {
            list_del(&tmp->list);
            free(tmp);
            return RET_OK;
        }
    }

    return RET_ERROR;
}

/*
 * description: 指令执行
 * str: 指向待解析字符串
 * len: 待解析字符串长度
 * return: 0 代表成功，其他值代表失败
 */
int AtCmdExec(const char *str, int len, SendToPc toPc)
{
    CHECK_RETURN(str != NULL, RET_INVALID_PARAM);
    CHECK_RETURN(len > 0, RET_INVALID_PARAM);

    char *buf = (char *)calloc(1, len + 1);
    CHECK_RETURN(buf != NULL, RET_MALLOC_ERROR);

    int ret = strncpy_s(buf, len + 1, str, len);
    CHECK_RETURN(ret == RET_OK, RET_INVALID_PARAM);

    /* 去掉命令尾部的 \r\n 或 \n */
    buf[strcspn(buf, "\r\n")] = 0;

    ret = RET_NOT_EXISTS;
    char *left = NULL;
    char *prefix = strtok_r(buf, PREFIX_DILM, &left);
    if (prefix == NULL) {
        LOG_E("prefix is null");
        ret = RET_ERROR;
        goto exit;
    }

    struct list_head *pos = NULL;
    list_for_each(pos, &g_cmdList)
    {
        AtCmdNode *tmp = list_entry(pos, AtCmdNode, list);
        if (strcmp(buf, tmp->cmd.str) != 0) {
            continue;
        }

        if (tmp->cmd.cb == NULL) {
            LOG_E("cmd cb is null");
            ret = RET_ERROR;
            goto exit;
        }

        int argc = 0;
        char *argv[MAX_PARAM_CNT] = { 0 };

        while (argc < MAX_PARAM_CNT) {
            if ((argv[argc] = strtok_r(NULL, PARAM_DILM, &left)) == NULL) {
                break;
            }

            if (++argc >= MAX_PARAM_CNT && strtok_r(NULL, PARAM_DILM, &left) != NULL) {
                LOG_E("too many args");
                ret = RET_ERROR;
                goto exit;
            }
        }

        ret = tmp->cmd.cb(argc, argv, toPc);

        break;
    }

exit:
    if (buf != NULL) {
        free(buf);
    }

    return ret;
}

int AtCmdInit(void)
{
    struct list_head *pos = NULL;
    list_for_each(pos, &g_moduleList)
    {
        ModuleNode *tmp = list_entry(pos, ModuleNode, list);

        for (int i = 0; i < tmp->len; ++i) {
            int ret = AtCmdRegister(tmp->cmds[i]);
            if (ret != RET_OK) {
                LOG_E("register fail");
                continue;
            }
        }
    }

    return RET_OK;
}

int AtCmdDeInit(void)
{
    struct list_head *pos = NULL;
    list_for_each(pos, &g_moduleList)
    {
        ModuleNode *tmp = list_entry(pos, ModuleNode, list);

        for (int i = 0; i < tmp->len; ++i) {
            int ret = AtCmdUnregister(tmp->cmds[i]);
            if (ret != RET_OK) {
                LOG_E("unregister fail");
                continue;
            }
        }
    }

    return RET_OK;
}