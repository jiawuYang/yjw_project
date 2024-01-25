#include "cmd_handle.h"
#include "AVCodecLib.h"
#include "assert_check.h"
#include "ats_avcodec.h"
#include "ats_battery.h"
#include "ats_button.h"
#include "ats_hw_ctrl.h"
#include "ats_night_monitor.h"
#include "ats_pir.h"
#include "ats_ptz.h"
#include "channel.h"
#include "did_api.h"
#include "error_code.h"
#include "hwfm_api.h"
#include "old_cmd.h"
#include "plat_net.h"
#include "plat_ptz.h"
#include "product_info_api.h"
#include "ptz_mngr.h"
#include "qrcode_api.h"
#include "sensor_api.h"
#include "tl_osal.h"
#include "utils.h"
#include "wifi_api.h"

#define TEST_TF_CARD_MAGIC "zsxdfcgb="
#define TEST_TF_CARD_FILE  "/tmp/sd/Factory/test_tf_card.txt"
#define MIN_DXCO           0
#define MAX_DXCO           128

#define CTRL_LED(func, op, ret)                                                                                        \
    do {                                                                                                               \
        if (strcmp(op, "ON") == 0) {                                                                                   \
            ret = func(LED_ON);                                                                                        \
        } else if (strcmp(op, "OFF") == 0) {                                                                           \
            ret = func(LED_OFF);                                                                                       \
        } else if (strcmp(op, "BLINK") == 0) {                                                                         \
            ret = func(LED_FLASH);                                                                                     \
        }                                                                                                              \
    } while (0)

static int GetFwVerHandle(int fd, int type, char *pData, int len)
{
    char pHwfmVer[64] = {0};

    int rslt = GetFmVer(pHwfmVer);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "FWVER_%s", pHwfmVer);

    return RET_OK;
}

static int GetHwVerHandle(int fd, int type, char *pData, int len)
{
    int ver = -1;
    int rslt = GetHwVer(&ver);
    CHECK_RETURN(rslt == RET_OK && ver != -1, RET_ERROR);

    AT_RSP(fd, type, "HWVER_%d", ver);

    return RET_OK;
}

static int GetAtsVerHandle(int fd, int type, char *pData, int len)
{
    AT_RSP(fd, type, "%s", ATS_VER);

    return RET_OK;
}

static int PcbSnReadHandle(int fd, int type, char *pData, int len)
{
    char PcbSN[64] = {0};

    int rslt = ReadPcbSN(PcbSN);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    AT_RSP(fd, type, "PCBSN_%s", PcbSN);

    return RET_OK;
}

static int PcbSnWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PCB_SN_WRITE) == 0, RET_INVALID_PARAM);

    char *pcbSn = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pcbSn != NULL, RET_INVALID_PARAM);
    LOG_I("pcbSn: %s", pcbSn);

    int rslt = WritePcbSN(pcbSn, strlen(pcbSn) + 1);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    return RET_OK;
}

static int SnReadHandle(int fd, int type, char *pData, int len)
{
    char sn[128] = {0};

    int rslt = ReadInfoFromDid(DID_INFO_SN, sn, sizeof(sn));
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    AT_RSP(fd, type, "SN_%s", sn);

    return RET_OK;
}

static int SnWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_SN_WRITE) == 0, RET_INVALID_PARAM);

    char *sn = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(sn != NULL, RET_INVALID_PARAM);
    LOG_I("sn: %s", sn);

    int rslt = WriteInfoToDid(DID_INFO_SN, sn);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    return RET_OK;
}

static int WifiMacReadHandle(int fd, int type, char *pData, int len)
{
    char mac[18] = {0};

    int rslt = PLAT_NetGetMacString3("wlan0", mac);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "WifiMAC_%s", mac);

    return RET_OK;
}

static int WifiCCWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_CC_WRITE) == 0, RET_INVALID_PARAM);

    char *region = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(region != NULL, RET_INVALID_PARAM);

    LOG_I("region: %s", region);

    int rslt = WriteWifiCC(region);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int WifiCCReadHandle(int fd, int type, char *pData, int len)
{
    char countryCode[64] = {0};

    int rslt = ReadWifiCC(countryCode, sizeof(countryCode));
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    AT_RSP(fd, type, "WifiCC_%s", countryCode);

    return RET_OK;
}

static int WifiScanHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_SCAN) == 0, RET_INVALID_PARAM);

    char *ssid = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(ssid != NULL, RET_INVALID_PARAM);

    LOG_I("ssid: %s", ssid);

    int rssi = 0;
    int rslt = GetEssidRssiApi(ssid, &rssi);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "RSSI_%d", rssi);

    return RET_OK;
}

static int WifiRxStartHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_RX_START) == 0, RET_INVALID_PARAM);

    char *channel = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(channel != NULL, RET_INVALID_PARAM);

    char *is40M = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(is40M != NULL, RET_INVALID_PARAM);

    LOG_I("channel: %s, is40M: %s", channel, is40M);

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, IWPRIV_PATH " wlan0 start_rx %s,%s", channel, is40M);
    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int WifiRxStopHandle(int fd, int type, char *pData, int len)
{
    const char *cmd = IWPRIV_PATH " wlan0 stop_rx";
    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    FILE *fp = popen("dmesg | tail -5", "r");
    CHECK_RETURN(fp != NULL, RET_ERROR);

    /* [  293.682607] [atbm_log]:atbm_ioctl_stop_rx:rxSuccess:10741, FcsErr:3003, PlcpErr:1216 */
    char buf[1024] = {0};
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *str = strstr(buf, "rxSuccess:");
        if (str != NULL) {
            AT_RSP(fd, type, "%s", str);
            pclose(fp);
            return RET_OK;
        }
        bzero(buf, sizeof(buf));
    }

    pclose(fp);

    return RET_ERROR;
}

static int WifiTxStartHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_TX_START) == 0, RET_INVALID_PARAM);

    char *channel = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(channel != NULL, RET_INVALID_PARAM);

    char *rateId = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(rateId != NULL, RET_INVALID_PARAM);

    LOG_I("channel: %s, rateId: %s", channel, rateId);

    struct WiFiRateId {
        char *id;
        char *rateM;
    } rateTab[] = {
        {"11B", "11"},
        {"11G", "54"},
        {"11N", "6.5"},
    };

    char *rateM = NULL;
    for (int i = 0; i < sizeof(rateTab) / sizeof(struct WiFiRateId); ++i) {
        if (strcmp(rateId, rateTab[i].id) == 0) {
            rateM = rateTab[i].rateM;
        }
    }
    CHECK_RETURN(rateM != NULL, RET_INVALID_PARAM);

    /* iwpriv wlan0 start_tx channel,rate,len,is_40M,greedfiled */
    /* iwpriv wlan0 start_tx 1,65,1000,0,0 */

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, IWPRIV_PATH " wlan0 start_tx %s,%s,1000,0,0", channel, rateM);

    LOG_I("cmd: %s", cmd);

    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int WifiTxStopHandle(int fd, int type, char *pData, int len)
{
    const char *cmd = IWPRIV_PATH " wlan0 stop_tx";
    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int WifiAteModeHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_ATE_MODE) == 0, RET_INVALID_PARAM);

    char *param = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(param != NULL, RET_INVALID_PARAM);

    long enter = strtol(param, NULL, 10);
    LOG_I("enter: %ld", enter);

    int ret = RET_ERROR;
    if (enter == 0) {
        ret = system("ifconfig wlan0 down");
    } else if (enter == 1) {
        ret = system("ifconfig wlan0 up");
    } else {
        return RET_INVALID_PARAM;
    }

    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

static int PtzDirHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PTZ_DIR) == 0, RET_INVALID_PARAM);

    char *param = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(param != NULL, RET_INVALID_PARAM);

    const char *direction[] = {"L", "R", "U", "D"};
    int dir = RET_ERROR;
    for (int i = 0; i < sizeof(direction) / sizeof(const char *); ++i) {
        if (strcmp(param, direction[i]) == 0) {
            dir = i + 1;
        }
    }
    CHECK_RETURN(dir != RET_ERROR, RET_INVALID_PARAM);

    LOG_I("dir: %d", dir);

    int ret = ATS_PtzMove(dir, MOTOR_SPEED_FAST);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

static int PtzPosReadHandle(int fd, int type, char *pData, int len)
{
    int x = 0;
    int y = 0;
    int ret = ATS_PtzGetPos(&x, &y);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    AT_RSP(fd, type, "x: %d, y: %d", x, y);

    return RET_OK;
}

static int PtzPosWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PTZ_POS_Write) == 0, RET_INVALID_PARAM);

    char *pX = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pX != NULL, RET_INVALID_PARAM);

    char *pY = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pY != NULL, RET_INVALID_PARAM);

    int x = atoi(pX);
    int y = atoi(pY);

    LOG_I("x: %d, y: %d", x, y);

    int ret = ATS_PtzSetPos(x, y);
    CHECK_RETURN(ret >= 0, RET_ERROR);

    return RET_OK;
}

static int OldFlagReadHandle(int fd, int type, char *pData, int len)
{
    int oldFlag = 0;

    int rslt = CheckOldFlag();
    if (rslt == RET_OK) {
        oldFlag = 1;
    }

    AT_RSP(fd, type, "OldFlag_%d", oldFlag);

    return RET_OK;
}

static int AudioFlagWriteHandle(int fd, int type, char *pData, int len)
{
    int rslt = SaveAudioFlag();
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int AudioFlagReadHandle(int fd, int type, char *pData, int len)
{
    int audioFlag = 0;

    int rslt = CheckAudioFlag();
    if (rslt == RET_OK) {
        audioFlag = 1;
    }

    AT_RSP(fd, type, "AudioFlag_%d", audioFlag);

    return RET_OK;
}

static int VolumeHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_CTRL_VOLUME) == 0, RET_INVALID_PARAM);

    char *pVolume = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pVolume != NULL, RET_INVALID_PARAM);

    long volume = strtol(pVolume, NULL, 10);
    LOG_I("volume: %ld", volume);
    int rslt = SetAgingAudioVolume((char)volume);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int IRCutCtrlHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_IR_CUT_CTRL) == 0, RET_INVALID_PARAM);

    char *op = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(op != NULL, RET_INVALID_PARAM);

    char *pMaintain = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pMaintain != NULL, RET_INVALID_PARAM);

    int maintain = atoi(pMaintain);
    CHECK_RETURN(maintain == 0 || maintain == 1, RET_INVALID_PARAM);

    LOG_I("op: %s, maintain: %d", op, maintain);

    int ret = RET_ERROR;
    if (strcmp(op, "ON") == 0) {
        ret = AtsSetDayNightMode(1, maintain);
    } else if (strcmp(op, "OFF") == 0) {
        ret = AtsSetDayNightMode(0, maintain);
    }
    CHECK_RETURN(ret == RET_OK, ret);

    return RET_OK;
}

static int IrLedCtrlHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_IR_LED_CTRL) == 0, RET_INVALID_PARAM);

    char *brightness = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(brightness != NULL, RET_INVALID_PARAM);

    char *op = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(op != NULL, RET_INVALID_PARAM);

    LOG_I("brightness: %s, op: %s", brightness, op);

    int ret = RET_ERROR;
    if (strcmp(op, "ON") == 0) {
        ret = HwCtrl_SetIRLED(1);
    } else if (strcmp(op, "OFF") == 0) {
        ret = HwCtrl_SetIRLED(0);
    }
    CHECK_RETURN(ret == RET_OK, ret);

    return RET_OK;
}

static int LedCtrlHandle(int fd, int type, char *pData, int len)
{
    /* param: AT+LED_RED_100_ON */
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_LED_CTRL) == 0, RET_INVALID_PARAM);

    char *color = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(color != NULL, RET_INVALID_PARAM);

    char *brightness = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(brightness != NULL, RET_INVALID_PARAM);

    char *op = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(op != NULL, RET_INVALID_PARAM);

    LOG_I("color: %s, brightness: %s, op: %s", color, brightness, op);

    int ret = RET_ERROR;
    if (strcmp(color, "RED") == 0) {
        CTRL_LED(HwCtrl_SetRedLED, op, ret);
    } else if (strcmp(color, "GREEN") == 0 || strcmp(color, "BLUE") == 0) {
        CTRL_LED(HwCtrl_SetGreenLED, op, ret);
    }
    CHECK_RETURN(ret == RET_OK, ret);

    return RET_OK;
}

int DidReadHandle(int fd, int type, char *pData, int len)
{
    char pDid[128] = {0};

    int rslt = ReadDidApi(pDid);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "DID=%s", pDid);

    return RET_OK;
}

int DidWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, "=", &pLeft);
    CHECK_RETURN(strcmp(head, AT_DID_WRITE) == 0, RET_INVALID_PARAM);

    char *did = strtok_r(NULL, "=", &pLeft);
    CHECK_RETURN(did != NULL, RET_INVALID_PARAM);

    LOG_I("did: %s", did);

    int rslt = WriteDidApi(did);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

int WifiMacWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_MAC_WRITE) == 0, RET_INVALID_PARAM);

    char *mac = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(mac != NULL, RET_INVALID_PARAM);
    LOG_I("mac: %s", mac);

    /* 写入 WiFi 芯片 */
    int rslt = write_wifimac_api(mac);
    CHECK_RETURN(rslt == RET_OK, rslt);

    /* 写入 DID 分区 */
    rslt = WriteInfoToDid(DID_INFO_MAC, mac);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    return RET_OK;
}

static int KeyTestHandle(int fd, int type, char *pData, int len)
{
    int key1 = -1;

    int rslt = ReadResetButton(&key1);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "Key1: %d", key1);

    return RET_OK;
}

static int RecordStartHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_RECORD_START) == 0, RET_INVALID_PARAM);

    int rslt = RecordAudio(true);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int RecordStopHandle(int fd, int type, char *pData, int len)
{
    int rslt = RecordAudio(false);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int PlayStartHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PLAY_START) == 0, RET_INVALID_PARAM);

    int rslt = PlayAudioFileStart();
    if (rslt != RET_OK) {
        if (rslt == RET_NOT_EXISTS) {
            AT_RSP(fd, type, "file not exist!");
        } else {
            AT_RSP(fd, type, "speaker or mic busy!");
        }
        return RET_ERROR;
    }

    return RET_OK;
}

static int PlayStopHandle(int fd, int type, char *pData, int len)
{
    int rslt = PlayAudioFileStop(true);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int GetRecordFileHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_GET_RECORD_FILE) == 0, RET_INVALID_PARAM);

    char *channel = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(channel != NULL, RET_INVALID_PARAM);

    char *ip = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(ip != NULL, RET_INVALID_PARAM);

    const char *fileName = NULL;
    if (strcmp(channel, "L") == 0) {
        fileName = REC_AUDIO_LEFT_FILENAME;
    } else if (strcmp(channel, "R") == 0) {
        fileName = REC_AUDIO_RIGHT_FILENAME;
    }
    CHECK_RETURN(fileName != NULL, RET_ERROR);

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, "cd %s;tftp -p -l %s %s", REC_AUDIO_FILE_PATH, fileName, ip);

    LOG_I("cmd: %s", cmd);

    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int DelRecordFileHandle(int fd, int type, char *pData, int len)
{
    int rslt = system("rm -f " REC_AUDIO_FILE_PATH REC_PCM_FILE_NAME);
    CHECK_RETURN(rslt == RET_OK, rslt);

    rslt = system("rm -f " REC_AUDIO_FILE_PATH REC_AUDIO_LEFT_FILENAME);
    CHECK_RETURN(rslt == RET_OK, rslt);

    rslt = system("rm -f " REC_AUDIO_FILE_PATH REC_AUDIO_RIGHT_FILENAME);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int WifiConnectHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_WIFI_CONNECT) == 0, RET_INVALID_PARAM);

    char *ssid = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(ssid != NULL, RET_INVALID_PARAM);

    char *password = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(password != NULL, RET_INVALID_PARAM);

    LOG_I("ssid: %s, password: %s", ssid, password);

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, "chmod u+x /tmp/wificonnect.sh && /tmp/wificonnect.sh %s WPA %s", ssid, password);
    LOG_I("cmd: %s", cmd);

    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    OS_SleepMs(2000);

    int isConnected = 0;
    rslt = PLAT_WifiGetInfo(&isConnected, NULL, NULL);
    if ((rslt == 0) && (isConnected == 1)) {
        OS_SleepMs(2000);
        return RET_OK;
    }

    LOG_W("rslt: %d, isConnected: %d", rslt, isConnected);

    return RET_ERROR;
}

static int WifiIPReadHandle(int fd, int type, char *pData, int len)
{
    char ip[64] = {0};
    int rslt = PLAT_NetGetIpv4("wlan0", ip);
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "IP_%s", ip);

    return RET_OK;
}

static int AtsEnableHandle(int fd, int type, char *pData, int len)
{
    int rslt = system("rm -rf /home/nvram/system/.ats_disable");
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "ATS enabled, effective on next boot");

    LOG_W("ATS enabled, effective on next boot");

    return RET_OK;
}

static int AtsDisableHandle(int fd, int type, char *pData, int len)
{
    int rslt = system("touch /home/nvram/system/.ats_disable");
    CHECK_RETURN(rslt == RET_OK, rslt);

    AT_RSP(fd, type, "ATS disabled, effective on next boot");

    LOG_W("ATS disabled, effective on next boot");

    return RET_OK;
}

static int AutoNightVisionHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_AUTO_NIGHT_VISION) == 0, RET_INVALID_PARAM);

    char *enable = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(enable != NULL, RET_INVALID_PARAM);

    LOG_I("auto night vision enable: %s", enable);

    NightMode_e mode = NIGHT_MODE_MAX;

    if (strcmp(enable, "1") == 0) {
        mode = NIGHT_MODE_AUTO;
    } else if (strcmp(enable, "0") == 0) {
        mode = NIGHT_MODE_MANUAL;
    }

    int rslt = AtsNightMonitorSetMode(mode);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

static int AutoPlayRecordHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PLAY_RECORD) == 0, RET_INVALID_PARAM);

    char *mic = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(mic != NULL, RET_INVALID_PARAM);

    CHECK_RETURN(strcmp(mic, "L") == 0 || strcmp(mic, "R") == 0, RET_INVALID_PARAM);

    int rslt = PlayRecordStart(mic);
    if (rslt != RET_OK) {
        if (rslt == RET_NOT_EXISTS) {
            AT_RSP(fd, type, "file not exist!");
        } else {
            AT_RSP(fd, type, "speaker or mic busy!");
        }
        return RET_ERROR;
    }

    return RET_OK;
}

static int PushAudioFileHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_PUSH_AUDIO_FILE) == 0, RET_INVALID_PARAM);

    char *ip = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(ip != NULL, RET_INVALID_PARAM);

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, "cd %s;tftp %s -g -r %s", ATS_TEST_SPEAKER_FILE_PATH, ip, ATS_TEST_SPEAKER_FILE_NAME);

    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

int GetSensorIdHandle(int fd, int type, char *pData, int len)
{
    char buf[128] = {0};
    int ret = AVC_VidGetSensorID(0, (void *)buf);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    LOG_I("buf: %s", buf);

    /* m00_b_sc3338 4-0030-3 */
    char *pLeft = NULL;
    char *head = strtok_r(buf, "_", &pLeft);
    CHECK_RETURN(head != NULL, RET_ERROR);

    char *placeHolder = strtok_r(NULL, "_", &pLeft);
    CHECK_RETURN(placeHolder != NULL, RET_ERROR);

    char *tmp = strtok_r(NULL, "_", &pLeft);
    CHECK_RETURN(tmp != NULL, RET_ERROR);

    char *id = strtok_r(tmp, " ", &pLeft);
    CHECK_RETURN(id != NULL, RET_ERROR);

    AT_RSP(fd, type, "SensorId_%s", id);

    return RET_OK;
}

int TestTfcardHandle(int fd, int type, char *pData, int len)
{
    bool testOk = false;

    FILE *file = fopen(TEST_TF_CARD_FILE, "wb+");
    if (file == NULL) {
        goto exit;
    }

    int ret = fwrite(TEST_TF_CARD_MAGIC, strlen(TEST_TF_CARD_MAGIC), 1, file);
    if (ret != 1) {
        goto exit;
    }

    fseek(file, 0, SEEK_SET);
    char buf[32] = {0};
    ret = fread(buf, strlen(TEST_TF_CARD_MAGIC), 1, file);
    LOG_I("ret = %d, buf = %s", ret, buf);
    if (ret != 1) {
        goto exit;
    }

    if (strcmp(TEST_TF_CARD_MAGIC, buf) == 0) {
        testOk = true;
    }

exit:
    if (file != NULL) {
        fclose(file);
    }

    if (testOk) {
        AT_RSP(fd, type, "TFTest_OK");
        return RET_OK;
    } else {
        AT_RSP(fd, type, "TFTest_NG");
        return RET_ERROR;
    }
}

int FreqDeviWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_FREQ_DEVI_WRITE) == 0, RET_INVALID_PARAM);

    char *pDcxo = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pDcxo != NULL, RET_INVALID_PARAM);
    int dcxo = atoi(pDcxo);
    CHECK_RETURN(dcxo >= MIN_DXCO && dcxo <= MAX_DXCO, RET_INVALID_PARAM);

    char *pSaveonPwrOff = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(pSaveonPwrOff != NULL, RET_INVALID_PARAM);
    int saveOnPwrOff = atoi(pSaveonPwrOff);
    CHECK_RETURN(saveOnPwrOff == 0 || saveOnPwrOff == 1, RET_INVALID_PARAM);

    LOG_I("dcxo: %d, saveOnPwrOff: %d", dcxo, saveOnPwrOff);

    char cmd[256] = {0};
    SNPRINTF(cmd, 256, IWPRIV_PATH " wlan0 common setEfuse_dcxo,%d,%d", dcxo, saveOnPwrOff);
    int rslt = system(cmd);
    CHECK_RETURN(rslt == RET_OK, rslt);

    return RET_OK;
}

int FreqDeviReadHandle(int fd, int type, char *pData, int len)
{
    int rslt = system(IWPRIV_PATH " wlan0 common getEfuse");
    CHECK_RETURN(rslt == RET_OK, rslt);

    FILE *fp = popen("dmesg | tail -5", "r");
    CHECK_RETURN(fp != NULL, RET_ERROR);

    /* [   16.642926] [atbm_log]:Get efuse data is [1,59,26,26,29,7,0,0,8c:85:80:a3:02:51] */
    char buf[1024] = {0};
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *position = strstr(buf, "Get efuse data is [");
        if (position == NULL) {
            bzero(buf, sizeof(buf));
            continue;
        }
        int placeHolder = 0;
        int dcxo = 0;
        sscanf(position, "Get efuse data is [%d,%d,", &placeHolder, &dcxo);
        AT_RSP(fd, type, "FreqDevi_%d", dcxo);
        pclose(fp);
        return RET_OK;
    }

    pclose(fp);
    return RET_ERROR;
}

int ResetHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_RESET) == 0, RET_INVALID_PARAM);

    LOG_W("do reset......");

    system("rm -rf /home/nvram/config/*");
    system("rm -rf /tmp/sd/record/*");
    system("sync");
    system("reboot");

    return RET_OK;
}

static int CpuIdReadHandle(int fd, int type, char *pData, int len)
{
    FILE *fp = popen("cat /proc/cpuinfo |grep Serial | awk -F \":\" '{print $2}'", "r");
    CHECK_RETURN(fp != NULL, RET_ERROR);

    char buf[1024] = {0};
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char *position = strstr(buf, " ");
        if (position == NULL) {
            bzero(buf, sizeof(buf));
            continue;
        }
        char id[32] = {0};
        sscanf(position, " %s", id);
        AT_RSP(fd, type, "CpuId_%s", id);
        pclose(fp);
        return RET_OK;
    }

    pclose(fp);

    return RET_ERROR;
}

static int AILicenseReadHandle(int fd, int type, char *pData, int len)
{
    char license[128] = {0};

    int rslt = ReadInfoFromDid(DID_INFO_AI_LICENSE, license, sizeof(license));
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    AT_RSP(fd, type, "AILicense_%s", license);

    return RET_OK;
}

static int AILicenseWriteHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_AI_LICENSE_WRITE) == 0, RET_INVALID_PARAM);

    char *license = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(license != NULL, RET_INVALID_PARAM);
    LOG_I("license: %s", license);

    int rslt = WriteInfoToDid(DID_INFO_AI_LICENSE, license);
    CHECK_RETURN(rslt == RET_OK, RET_ERROR);

    return RET_OK;
}

static int UartEnableHandle(int fd, int type, char *pData, int len)
{
    char *pLeft = NULL;
    char *head = strtok_r(pData, PARAM_DILM, &pLeft);
    CHECK_RETURN(strcmp(head, AT_UART_ENABLE) == 0, RET_INVALID_PARAM);

    char *enable = strtok_r(NULL, PARAM_DILM, &pLeft);
    CHECK_RETURN(enable != NULL, RET_INVALID_PARAM);
    LOG_I("enable: %s", enable);

    struct UartGpioCtrl {
        char *enable;
        char *cmd;
    } cmdTab[] = {
        {"0", "io -4 0xFF538008 0x70000000"},
        {"1", "io -4 0xFF538008 0x70002000"},
    };

    char *cmd = NULL;
    for (int i = 0; i < sizeof(cmdTab) / sizeof(struct UartGpioCtrl); ++i) {
        if (strcmp(enable, cmdTab[i].enable) == 0) {
            cmd = cmdTab[i].cmd;
        }
    }
    CHECK_RETURN(cmd != NULL, RET_INVALID_PARAM);

    LOG_I("cmd: %s", cmd);

    int ret = system(cmd);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

cmd_handles_s g_cmd_handles[] = {
    /* 兼容旧产品 */
    {TL_CMD_AT_EXITATS, ExitAtsModeHandle},
    {TL_CMD_AT_FMDATE, ReadFMDateHandle},
    {TL_CMD_AT_READPD, ReadPdHandle},
    {TL_CMD_AT_WRITEPD, WritePdHandle},
    {TL_CMD_AT_READWIFIMAC, WifiMacReadHandle},
    {TL_CMD_AT_WIFIVER, GetWifiVerHandle},
    {TL_CMD_AT_WIFISCAN_SSID, TestWifiScanHandle},
    {TL_CMD_AT_WIRED_IP, TestWiredHandle},
    {TL_CMD_AT_SPK, test_speaker_handle},
    {TL_CMD_AT_MIC, test_mic_handle},
    {TL_CMD_AT_IRCUT_ON2, CtrlIRCutHandle},
    {TL_CMD_AT_PTZTEST, TestPtzHandle},
    {TL_CMD_AT_SCANQRCODE, ScanQRCodeHandle},
    {TL_CMD_AT_SENSORLV, get_sensorlv_handle},
    {TL_CMD_AT_SENSORID, GetSensorIdHandle},
    {TL_CMD_AT_LIGHTSENSORVAL, GetLightSensorValHandle},
    {TL_CMD_AT_WRITEWIFIMAC, WifiMacWriteHandle},
    {TL_CMD_AT_CALL_BUTTON, call_button_handle},
    {TL_CMD_AT_SAVEAUDIO, SaveRecAudioHandle},
    {TL_CMD_AT_WIFIMODULECTRL, WIFIModuleCtrlHandle},
    {TL_CMD_AT_PTZ_DIR_U, TestPtzHandle},
    {TL_CMD_AT_PTZ_DIR_D, TestPtzHandle},
    {TL_CMD_AT_PTZ_DIR_L, TestPtzHandle},
    {TL_CMD_AT_PTZ_DIR_R, TestPtzHandle},
    {TL_CMD_AT_LED_ON, ctrl_led_handle},
    {TL_CMD_AT_LED_OFF, ctrl_led_handle},
    {TL_CMD_AT_LEDR_ON, ctrl_led_handle},
    {TL_CMD_AT_LEDR_OFF, ctrl_led_handle},
    {TL_CMD_AT_LEDG_ON, ctrl_led_handle},
    {TL_CMD_AT_LEDG_OFF, ctrl_led_handle},
    {TL_CMD_AT_LEDW_ON, ctrl_whiteled_handle},
    {TL_CMD_AT_LEDW_OFF, ctrl_whiteled_handle},
    {TL_CMD_AT_MANDATE, AlgoMandateHandle},
    {TL_CMD_AT_SENSOR_TEST, test_sensor_handle},
    {TL_CMD_AT_PIR_TEST, TestPIRTriggerHandle},
    {TL_CMD_AT_GET_BATVOL, GetBatteryVolHandle},
    {TL_CMD_AT_WRITEPCBSN, PcbSnWriteHandle},
    {TL_CMD_AT_READPCBSN, PcbSnReadHandle},
    {TL_CMD_AT_CHECK_OLDFLAG, OldFlagReadHandle},
    /* ATS 模块标准化 */
    {AT_FW_VER, GetFwVerHandle},
    {AT_HW_VER, GetHwVerHandle},
    {AT_ATS_VER, GetAtsVerHandle},
    {AT_PCB_SN_WRITE, PcbSnWriteHandle},
    {AT_PCB_SN_READ, PcbSnReadHandle},
    {AT_SN_WRITE, SnWriteHandle},
    {AT_SN_READ, SnReadHandle},
    {AT_AUDIO_FLAG_WRITE, AudioFlagWriteHandle},
    {AT_AUDIO_FLAG_READ, AudioFlagReadHandle},
    {AT_DID_WRITE, DidWriteHandle},
    {AT_DID_READ, DidReadHandle},
    {AT_LED_CTRL, LedCtrlHandle},
    {AT_IR_LED_CTRL, IrLedCtrlHandle},
    {AT_IR_CUT_CTRL, IRCutCtrlHandle},
    {AT_OLD_FLAG_READ, OldFlagReadHandle},
    {AT_CTRL_VOLUME, VolumeHandle},
    {AT_RECORD_START, RecordStartHandle},
    {AT_RECORD_STOP, RecordStopHandle},
    {AT_PLAY_START, PlayStartHandle},
    {AT_PLAY_STOP, PlayStopHandle},
    {AT_GET_RECORD_FILE, GetRecordFileHandle},
    {AT_DEL_RECORD_FILE, DelRecordFileHandle},
    {AT_KEY_TEST, KeyTestHandle},
    {AT_WIFI_MAC_WRITE, WifiMacWriteHandle},
    {AT_WIFI_MAC_READ, WifiMacReadHandle},
    {AT_WIFI_CC_WRITE, WifiCCWriteHandle},
    {AT_WIFI_CC_READ, WifiCCReadHandle},
    {AT_WIFI_CONNECT, WifiConnectHandle},
    {AT_WIFI_SCAN, WifiScanHandle},
    {AT_WIFI_RX_START, WifiRxStartHandle},
    {AT_WIFI_RX_STOP, WifiRxStopHandle},
    {AT_WIFI_TX_START, WifiTxStartHandle},
    {AT_WIFI_TX_STOP, WifiTxStopHandle},
    {AT_WIFI_ATE_MODE, WifiAteModeHandle},
    {AT_PTZ_DIR, PtzDirHandle},
    {AT_PTZ_POS_READ, PtzPosReadHandle},
    {AT_PTZ_POS_Write, PtzPosWriteHandle},
    {AT_PUSH_AUDIO_FILE, PushAudioFileHandle},
    {AT_GET_SENSOR_ID, GetSensorIdHandle},
    {AT_TFTEST, TestTfcardHandle},
    {AT_FREQ_DEVI_WRITE, FreqDeviWriteHandle},
    {AT_FREQ_DEVI_READ, FreqDeviReadHandle},
    {AT_RESET, ResetHandle},
    {AT_CPU_ID_READ, CpuIdReadHandle},
    {AT_AI_LICENSE_WRITE, AILicenseWriteHandle},
    {AT_AI_LICENSE_READ, AILicenseReadHandle},
    {AT_WIFI_IP_READ, WifiIPReadHandle},
    {AT_ATS_ENABLE, AtsEnableHandle},
    {AT_ATS_DISABLE, AtsDisableHandle},
    {AT_AUTO_NIGHT_VISION, AutoNightVisionHandle},
    {AT_PLAY_RECORD, AutoPlayRecordHandle},
    /* 调试用指令，不提供给工厂 */
    {AT_UART_ENABLE, UartEnableHandle},
};

int g_cmd_handles_count = sizeof(g_cmd_handles) / sizeof(g_cmd_handles[0]);

int cmd_process_handle(int fd, int type, char *pData, int len)
{
    CHECK_RETURN(pData != NULL, RET_INVALID_PARAM);
    CHECK_RETURN(len > strlen("AT+"), RET_INVALID_PARAM);

    for (int i = 0; i < g_cmd_handles_count; i++) {
        if (strstr(pData, g_cmd_handles[i].pCmd) != NULL) {
            int ret = RET_ERROR;
            if (g_cmd_handles[i].func != NULL) {
                /* 把命令尾部的\r\n去掉 */
                char *buff = (char *)OS_Calloc(1, len);
                for (int i = 0; i < len; ++i) {
                    if (pData[i] == '\r' || pData[i] == '\n') {
                        break;
                    }
                    buff[i] = pData[i];
                }
                ret = g_cmd_handles[i].func(fd, type, buff, len);
                OS_Free(buff);
            }
            if (ret == RET_OK) {
                AT_RSP(fd, type, "%s", RSP_OK);
            } else {
                AT_RSP(fd, type, "%s", RSP_ERR);
            }
            return ret;
        }
    }

    AT_RSP(fd, type, "%s", RSP_NOT_FOUNT);
    return RET_ERROR;
}
