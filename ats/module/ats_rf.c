#include "assert_check.h"
#include "ats.h"
#include "error_code.h"
#include <stdlib.h>
#include <unistd.h>

#define AT_RF_TEST_ON      "TL_DUT_RFTEST_ON"
#define AT_RF_TEST_OFF     "TL_DUT_RFTEST_OFF"

#define AT_WIFI_TX_START   "TL_WIFI_TX_ON"
#define AT_WIFI_TX_STOP    "TL_WIFI_TX_OFF"
#define AT_WIFI_RX_START   "TL_SET_WIFI_RX"
#define AT_WIFI_RX_STOP    "TL_WIFI_RX_OFF"

#define AT_BT_TX_START     "TL_BT_TX_ON"
#define AT_BT_TX_STOP      "TL_BT_TX_OFF"
#define AT_BT_RX_START     "TL_SET_BT_RX"
#define AT_BT_RX_STOP      "TL_BT_RX_OFF"

#define AT_BLE_TX_START    "TL_BLE_TX_ON"
#define AT_BLE_TX_STOP     "TL_BLE_TX_OFF"
#define AT_BLE_RX_START    "TL_SET_BLE_RX"
#define AT_BLE_RX_STOP     "TL_BLE_RX_OFF"

#define AT_THREAD_TX_START "TL_THREAD_TX_ON"
#define AT_THREAD_TX_STOP  "TL_THREAD_TX_OFF"
#define AT_THREAD_RX_START "TL_SET_THREAD_RX"
#define AT_THREAD_RX_STOP  "TL_THREAD_RX_OFF"

#define AT_LORA_TX_START   "TL_LORA_TX_ON"
#define AT_LORA_TX_STOP    "TL_LORA_TX_OFF"
#define AT_LORA_RX_START   "TL_SET_LORA_RX"
#define AT_LORA_RX_STOP    "TL_LORA_RX_OFF"

static ModuleNode *g_moduleNode = NULL;
extern list_head_t g_moduleList;

static int RfTestOnHandle(int argc, char *argv[], SendToPc toPc)
{
    return RET_OK;
}

static int RfTestOffHandle(int argc, char *argv[], SendToPc toPc)
{
    return RET_OK;
}

static int WifiTxStartHandle(int argc, char *argv[], SendToPc toPc)
{
    return RET_OK;
}

static int WifiTxStopHandle(int argc, char *argv[], SendToPc toPc)
{
    return RET_OK;
}

static AtCmd g_cmds[] = {
    {AT_RF_TEST_ON, RfTestOnHandle},
    {AT_RF_TEST_OFF, RfTestOffHandle},
    {AT_WIFI_TX_START, WifiTxStartHandle},
    {AT_WIFI_TX_STOP, WifiTxStopHandle},
    // { AT_WIFI_RX_START, NULL },
    // { AT_WIFI_RX_STOP, NULL },

    // { AT_BT_TX_START, NULL },
    // { AT_BT_TX_STOP, NULL },
    // { AT_BT_RX_START, NULL },
    // { AT_BT_RX_STOP, NULL },

    // { AT_BLE_TX_START, NULL },
    // { AT_BLE_TX_STOP, NULL },
    // { AT_BLE_RX_START, NULL },
    // { AT_BLE_RX_STOP, NULL },

    // { AT_THREAD_TX_START, NULL },
    // { AT_THREAD_TX_STOP, NULL },
    // { AT_THREAD_RX_START, NULL },
    // { AT_THREAD_RX_STOP, NULL },

    // { AT_LORA_TX_START, NULL },
    // { AT_LORA_TX_STOP, NULL },
    // { AT_LORA_RX_START, NULL },
    // { AT_LORA_RX_STOP, NULL },
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