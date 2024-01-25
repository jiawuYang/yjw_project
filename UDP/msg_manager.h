/*********************************************************************************
*Copyright(C),2020-2030, Tonly Electronics Holdings Limited
*FileName:  msg_manager.h
*Author:
*Version:	V0.1
*Date:
*Description:
*History:
1.Date:		2020.10.14
Author:
Modification:
**********************************************************************************/
#ifndef __TONLY_ATS_MSG_MANAGER_H__
#define __TONLY_ATS_MSG_MANAGER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <termios.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
#define SERVER_PORT 60001
void MsgManagerInit(void);

#ifdef __cplusplus
}
#endif
#endif
