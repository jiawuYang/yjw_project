/*********************************************************************************
*Copyright(C),2020-2030, Tonly Electronics Holdings Limited
*FileName:  cmd_handle.h
*Author:
*Version:	V0.1
*Date:
*Description:
*History:
1.Date:		2020.10.14
Author:
Modification:
**********************************************************************************/
#ifndef __TONLY_ATS_CMD_HANDLE_H__
#define __TONLY_ATS_CMD_HANDLE_H__

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *pCmd;
    int (*func)(int fd, int type, char *pData, int len);
} cmd_handles_s;

/* TL_CMD_ 开头的为了兼容旧产品 */
#define TL_CMD_AT_EXITATS        "AT+EXITATS"
#define TL_CMD_AT_FMDATE         "AT+FMDate"
#define TL_CMD_AT_READPD         "AT+PDRead"
#define TL_CMD_AT_WRITEPD        "AT+PDWrite_"
#define TL_CMD_AT_READWIFIMAC    "AT+ReadWifiMac"
#define TL_CMD_AT_WIFIVER        "AT+WIFIVER"
#define TL_CMD_AT_WIFISCAN_SSID  "AT+WIFISCAN_" // "AT+WIFISCAN_SSID"
#define TL_CMD_AT_WIRED_IP       "AT+WireIp"
#define TL_CMD_AT_SPK            "AT+SPK"
#define TL_CMD_AT_MIC            "AT+MIC"
#define TL_CMD_AT_IRCUT_ON2      "AT+IRCUT_OO"
#define TL_CMD_AT_IRLED_ON       "AT+IRLED_ON"
#define TL_CMD_AT_IRLED_OFF      "AT+IRLED_OFF"
#define TL_CMD_AT_PTZTEST        "AT+PTZTEST"
#define TL_CMD_AT_SCANQRCODE     "AT+QRCODESCAN"
#define TL_CMD_AT_SENSORLV       "AT+SensorLv"
#define TL_CMD_AT_SENSORID       "AT+SENSORID"
#define TL_CMD_AT_LIGHTSENSORVAL "AT+LightSensorVal"
#define TL_CMD_AT_WRITEWIFIMAC   "AT+WriteWifiMac_"
#define TL_CMD_AT_CALL_BUTTON    "AT+Call"
#define TL_CMD_AT_SAVEAUDIO      "AT+SaveAudio"
#define TL_CMD_AT_WIFIMODULECTRL "AT+WIFICtrl "
#define TL_CMD_AT_PTZ_DIR_U      "AT+PTZDIR_U"
#define TL_CMD_AT_PTZ_DIR_D      "AT+PTZDIR_D"
#define TL_CMD_AT_PTZ_DIR_L      "AT+PTZDIR_L"
#define TL_CMD_AT_PTZ_DIR_R      "AT+PTZDIR_R"
#define TL_CMD_AT_LED_ON         "AT+LED_ON"
#define TL_CMD_AT_LED_OFF        "AT+LED_OFF"
#define TL_CMD_AT_LEDR_ON        "AT+LEDR_ON"
#define TL_CMD_AT_LEDR_OFF       "AT+LEDR_OFF"
#define TL_CMD_AT_LEDG_ON        "AT+LEDG_ON"
#define TL_CMD_AT_LEDG_OFF       "AT+LEDG_OFF"
#define TL_CMD_AT_LEDW_ON        "AT+LEDW_ON"
#define TL_CMD_AT_LEDW_OFF       "AT+LEDW_OFF"
#define TL_CMD_AT_MANDATE        "AT+Mandate_"
#define TL_CMD_AT_SENSOR_TEST    "AT+SensorTest"
#define TL_CMD_AT_PIR_TEST       "AT+PIRTest"
#define TL_CMD_AT_GET_BATVOL     "AT+GetBatVol"
#define TL_CMD_AT_WRITEPCBSN     "AT+PCBWrite_"
#define TL_CMD_AT_READPCBSN      "AT+PCBRead"
#define TL_CMD_AT_CHECK_OLDFLAG  "AT+OLDFLAG"

/* ATS 模块标准化 */
#define AT_PCB_SN_WRITE      "AT+PCBSNWrite"
#define AT_PCB_SN_READ       "AT+PCBSNRead"
#define AT_SN_WRITE          "AT+SNWrite"
#define AT_SN_READ           "AT+SNRead"
#define AT_AUDIO_FLAG_WRITE  "AT+AudioFlagWrite"
#define AT_AUDIO_FLAG_READ   "AT+AudioFlagRead"
#define AT_LED_CTRL          "AT+LED"
#define AT_IR_LED_CTRL       "AT+IRLED"
#define AT_IR_CUT_CTRL       "AT+IRCUT"
#define AT_OLD_FLAG_READ     "AT+OldFlagRead"
#define AT_CTRL_VOLUME       "AT+Volume"
#define AT_RECORD_START      "AT+RecordStart"
#define AT_PLAY_START        "AT+PlayStart"
#define AT_GET_RECORD_FILE   "AT+GetRecordFile"
#define AT_DEL_RECORD_FILE   "AT+DelRecordFile"
#define AT_DID_WRITE         "AT+DIDWrite"
#define AT_PLAY_STOP         "AT+PlayStop"
#define AT_FW_VER            "AT+FWVER"
#define AT_HW_VER            "AT+HWVER"
#define AT_ATS_VER           "AT+ATSVER"
#define AT_DID_READ          "AT+DIDRead"
#define AT_RECORD_STOP       "AT+RecordStop"
#define AT_KEY_TEST          "AT+KeyTest"
#define AT_WIFI_CONNECT      "AT+WifiConnect"
#define AT_WIFI_MAC_WRITE    "AT+WifiMACWrite"
#define AT_WIFI_MAC_READ     "AT+WifiMACRead"
#define AT_WIFI_CC_WRITE     "AT+WifiCCWrite"
#define AT_WIFI_CC_READ      "AT+WifiCCRead"
#define AT_WIFI_SCAN         "AT+WifiScan"
#define AT_WIFI_RX_START     "AT+WifiRxStart"
#define AT_WIFI_RX_STOP      "AT+WifiRxStop"
#define AT_WIFI_TX_START     "AT+WifiTxStart"
#define AT_WIFI_TX_STOP      "AT+WifiTxStop"
#define AT_WIFI_ATE_MODE     "AT+WifiAteMode"
#define AT_PTZ_DIR           "AT+PtzDir"
#define AT_PTZ_POS_READ      "AT+PtzPosRead"
#define AT_PTZ_POS_Write     "AT+PtzPosWrite"
#define AT_PUSH_AUDIO_FILE   "AT+PushAudioFile"
#define AT_GET_SENSOR_ID     "AT+SensorID"
#define AT_TFTEST            "AT+TFTest"
#define AT_FREQ_DEVI_WRITE   "AT+FreqDeviWrite"
#define AT_FREQ_DEVI_READ    "AT+FreqDeviRead"
#define AT_RESET             "AT+Reset"
#define AT_CPU_ID_READ       "AT+CpuIdRead"
#define AT_AI_LICENSE_WRITE  "AT+AILicenseWrite"
#define AT_AI_LICENSE_READ   "AT+AILicenseRead"
#define AT_WIFI_IP_READ      "AT+WifiIPRead"
#define AT_ATS_ENABLE        "AT+AtsEnable"
#define AT_ATS_DISABLE       "AT+AtsDisable"
#define AT_AUTO_NIGHT_VISION "AT+AutoNightVision"
#define AT_PLAY_RECORD       "AT+PlayRecord"

/* 调试用指令，不提供给工厂 */
#define AT_UART_ENABLE "AT+UartEnable"

extern cmd_handles_s g_cmd_handles[];
extern int g_cmd_handles_count;

#define SERIAL_TPYE 0
#define UDP_TYPE    1
int cmd_process_handle(int fd, int type, char *pData, int len);

#define ATS_VER         "ATSVER_V1.13"
#define RSP_ERR         "fail"
#define RSP_OK          "success"
#define RSP_NOT_FOUNT   "cmd not found"
#define AT_RSP_BUFF_LEN 256
#define PARAM_DILM      "_"

#define AT_RSP(fd, type, fmt, args...)                                                                                 \
    do {                                                                                                               \
        char buff[AT_RSP_BUFF_LEN] = {0};                                                                              \
        SNPRINTF(buff, AT_RSP_BUFF_LEN, "\r\n" fmt "\r\n", ##args);                                                    \
        channel_data_send(fd, type, buff, strlen(buff));                                                               \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif
