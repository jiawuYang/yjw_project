#include "msg_manager.h"
#include "channel.h"
#include "cmd_handle.h"
#include "log.h"
#include "tl_osal.h"
#include <arpa/inet.h>

struct sockaddr_in server_addr;
static char *g_uartThreadName = "AtsUartThread";
static char *g_udpThreadName = "AtsUdpThread";

static void *UartRecvThread(void *arg)
{
    char buffer[256] = {0};
    int fd = channel_init();
    OS_Thread_SetName(g_uartThreadName);

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int len = channel_data_recv(fd, buffer, sizeof(buffer));
        LOG_D("chanel recv cmd");
        if (3 >= len) {
            continue;
        }
        cmd_process_handle(fd, SERIAL_TPYE, buffer, len);
    }

    LOG_E("UartRecvThread exit");
    return NULL;
}

static void *UdpRecvThread(void *arg)
{
    int opt = 1;
    char msgbuf[256] = {0};
    socklen_t socket_len = sizeof(server_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    OS_Thread_SetName(g_udpThreadName);

    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketfd < 0) {
        LOG_E("creat socket error:%s", strerror(errno));
        return NULL;
    }

    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        LOG_E("bind socket error:%s", strerror(errno));
        return NULL;
    }

    while (1) {
        memset(msgbuf, 0, sizeof(msgbuf));
        int recv_len = recvfrom(socketfd, msgbuf, sizeof(msgbuf), 0, (struct sockaddr *)&server_addr, &socket_len);
        if (recv_len <= 0) {
            LOG_W("recv data error !");
            continue;
        }
        if (msgbuf[0] == 'A' && msgbuf[1] == 'T' && msgbuf[2] == '+') {
            cmd_process_handle(socketfd, UDP_TYPE, msgbuf, recv_len);
        }
    }

    close(socketfd);
    LOG_E("UdpRecvThread exit");
    return NULL;
}

void MsgManagerInit(void)
{
    void *uartThreadId = NULL;
    void *udpThreadId = NULL;

    OS_Thread_CreateEx(g_uartThreadName, UartRecvThread, NULL, 0, 0, &uartThreadId);
    OS_Thread_CreateEx(g_udpThreadName, UdpRecvThread, NULL, 0, 0, &udpThreadId);
}
