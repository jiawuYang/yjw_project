#include "assert_check.h"
#include "ats.h"
#include "dlist.h"
#include "error_code.h"
#include <stdlib.h>
#include <unistd.h>

#define AT_ATS_IN             "TL_ATS_IN"
#define AT_SW_VER             "TL_DUT_SW_VER_IMX8MP"
#define AT_PSN_WRITE          "TL_SET_PSN"
#define AT_PSN_READ           "TL_GET_PSN"
#define AT_WIFI_MAC_WRITE     "TL_SET_WIFIMAC"
#define AT_WIFI_MAC_READ      "TL_GET_WIFIMAC"
#define AT_BT_MAC_WRITE       "TL_SET_BTMAC"
#define AT_BT_MAC_READ        "TL_GET_BTMAC"
#define AT_THREAD_MAC_WRITE   "TL_SET_THREADMAC"
#define AT_THREAD_MAC_READ    "TL_GET_THREADMAC"
#define AT_ETH_MAC_WRITE      "TL_SET_ETHMAC_X"
#define AT_ETH_MAC_READ       "TL_GET_ETHMAC_X"
#define AT_ETH_IP_READ        "TL_GET_ETH_X_IP"
#define AT_ETH_RATE_READ      "TL_GET_ETH_RATE_X"
#define AT_CHECK_USB          "TL_CHECK_USB"
#define AT_USB_RATE_READ      "TL_GET_USB_RATE"
#define AT_CHECK_IMX8         "TL_CHECK_IMX8_CAN_X"
#define AT_BACKLIGHT_ON       "TL_BACKLIGHT_ON"
#define AT_BACKLIGHT_OFF      "TL_BACKLIGHT_OFF"
#define AT_BACKLIGHT_WRITE    "TL_SET_BACKLIGHT=XX"
#define AT_PICTURE_WRITE      "TL_SET_PICTURE=X"
#define AT_VOLUME_WRITE       "TL_SET_VOL=XX"
#define AT_SPEAKER_ON         "TL_SPK_ALL_ON"
#define AT_SPEAKER_OFF        "TL_SPK_ALL_OFF"
#define AT_ADC_READ           "TL_GET_ADC_X"
#define AT_CHECK_LIGHT_SENSOR "TL_CHECK_LIGHT_SENSOR"
#define AT_FAN_SPEED_WRITE    "TL_SET_DRIVER_FAN_SPEED"
#define AT_FAN_SPEED_READ     "TL_GET_DRIVER_FAN_SPEED"
#define AT_CHECK_LTE          "TL_CHECK_LTE"

static ModuleNode *g_moduleNode = NULL;
extern list_head_t g_moduleList;

static int AtsInHandle(int argc, char *argv[], SendToPc toPc)
{
    toPc("TL_ATS_IN\r\n");
    return RET_OK;
}

static int SwVerHandle(int argc, char *argv[], SendToPc toPc)
{
    toPc("TL_DUT_SW_VER_IMX8MP=v1.0\r\n");
    return RET_OK;
}

static AtCmd g_cmds[] = {
    {AT_ATS_IN, AtsInHandle},
    {AT_SW_VER, SwVerHandle},
    // { AT_PSN_WRITE, NULL },
    // { AT_PSN_READ, NULL },
    // { AT_WIFI_MAC_WRITE, NULL },
    // { AT_WIFI_MAC_READ, NULL },
    // { AT_BT_MAC_WRITE, NULL },
    // { AT_BT_MAC_READ, NULL },
    // { AT_THREAD_MAC_WRITE, NULL },
    // { AT_THREAD_MAC_READ, NULL },
    // { AT_ETH_MAC_WRITE, NULL },
    // { AT_ETH_MAC_READ, NULL },
    // { AT_ETH_IP_READ, NULL },
    // { AT_ETH_RATE_READ, NULL },
    // { AT_CHECK_USB, NULL },
    // { AT_USB_RATE_READ, NULL },
    // { AT_CHECK_IMX8, NULL },
    // { AT_BACKLIGHT_ON, NULL },
    // { AT_BACKLIGHT_OFF, NULL },
    // { AT_BACKLIGHT_WRITE, NULL },
    // { AT_PICTURE_WRITE, NULL },
    // { AT_VOLUME_WRITE, NULL },
    // { AT_SPEAKER_ON, NULL },
    // { AT_SPEAKER_OFF, NULL },
    // { AT_ADC_READ, NULL },
    // { AT_CHECK_LIGHT_SENSOR, NULL },
    // { AT_FAN_SPEED_WRITE, NULL },
    // { AT_FAN_SPEED_READ, NULL },
    // { AT_CHECK_LTE, NULL },
};

static __attribute((constructor)) void CmdsInit()
{
    g_moduleNode = calloc(1, sizeof(ModuleNode));
    g_moduleNode->cmds = g_cmds;
    g_moduleNode->len = sizeof(g_cmds) / sizeof(AtCmd);

    list_add_tail(&g_moduleNode->list, &g_moduleList);
}

static __attribute((destructor)) void CmdsDeInit()
{
    list_del(&g_moduleNode->list);
    free(g_moduleNode);
}