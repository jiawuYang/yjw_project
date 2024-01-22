#include "ats_chn.h"
#include "assert_check.h"
#include "ats.h"
#include "error_code.h"
#include "log.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STD_IN_THREAD_NAME "AtStdInThread"
#define UDP_THREAD_NAME    "AtUdpThread"

static struct sockaddr_in g_srvAddr;
static int g_socket = 0;

static void *StdInRecvThread(void *arg)
{
    char buf[256] = { 0 };

    while (1) {
        memset(buf, 0, sizeof(buf));

        int ret = scanf("%s", buf);
        if (ret != 1) {
            continue;
        }

        if (buf[0] == 'T' && buf[1] == 'L' && buf[2] == '_') {
            int ret = AtCmdExec(buf, strlen(buf), printf);
            if (ret == RET_OK) {
                printf("SUCCESS\r\n");
            } else {
                printf("FAIL\r\n");
            }
        }
    }

    LOG_E("StdInRecvThread exit");
    return NULL;
}

int UdpSendToPc(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    char buf[256] = { 0 };
    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    int size = strlen(buf);

    sendto(g_socket, buf, size, 0, (struct sockaddr *)&g_srvAddr, sizeof(struct sockaddr));

    return size;
}

static void *UdpRecvThread(void *arg)
{
    int opt = 1;
    char buf[256] = { 0 };
    socklen_t socket_len = sizeof(g_srvAddr);

    g_srvAddr.sin_family = AF_INET;
    g_srvAddr.sin_port = htons(SERVER_PORT);
    g_srvAddr.sin_addr.s_addr = INADDR_ANY;

    g_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_socket < 0) {
        LOG_E("creat socket error");
        return NULL;
    }

    setsockopt(g_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(g_socket, (struct sockaddr *)&g_srvAddr, sizeof(g_srvAddr)) < 0) {
        LOG_E("bind socket error");
        return NULL;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        int len
            = recvfrom(g_socket, buf, sizeof(buf), 0, (struct sockaddr *)&g_srvAddr, &socket_len);
        if (len <= 0) {
            LOG_W("recv data error !");
            continue;
        }

        if (buf[0] == 'T' && buf[1] == 'L' && buf[2] == '_') {
            int ret = AtCmdExec(buf, len, UdpSendToPc);
            if (ret == RET_OK) {
                UdpSendToPc("SUCCESS\r\n");
            } else {
                UdpSendToPc("FAIL\r\n");
            }
        }
    }

    close(g_socket);
    LOG_E("UdpRecvThread exit");
    return NULL;
}

int ChnUdpInit(void)
{
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, UdpRecvThread, NULL);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

int ChnStdInInit(void)
{
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, StdInRecvThread, NULL);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}