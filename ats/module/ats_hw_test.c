#include "assert_check.h"
#include "ats.h"
#include "error_code.h"
#include "gpio.h"
#include "i2c.h"
#include "lcd.h"
#include "log.h"
#include "phy.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 下列指令只用于硬件测试阶段
#define AT_GPIO_READ         "TL_GPIO_READ"
#define AT_GPIO_WRITE        "TL_GPIO_WRITE"
#define AT_GPIO_MODE         "TL_GPIO_MODE"
#define AT_WIFI_CONN         "TL_WIFI_CONN"
#define AT_I2C_TEST          "TL_I2C_TEST"
#define AT_ETH_PHY_TEST      "TL_ETH_PHY_TEST"
#define AT_USB_TEST          "TL_USB_TEST"
#define AT_LCD_TEST          "TL_LCD_TEST"
#define AT_HALL_TEST         "TL_HALL_TEST"
#define AT_LIGHT_SENSOR_TEST "TL_LIGHT_SENSOR_TEST"

static ModuleNode *g_moduleNode = NULL;
extern list_head_t g_moduleList;

static int GpioReadHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 2, RET_INVALID_PARAM);

    int gpio = atoi(argv[0]);
    int io = atoi(argv[1]);

    int val = GetGpio(gpio, io);

    toPc("GPIO%d_IO%d_%s", gpio, io, val ? "high" : "low");

    return RET_OK;
}

static int GpioWriteHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 3, RET_INVALID_PARAM);

    int gpio = atoi(argv[0]);
    int io = atoi(argv[1]);
    int val = atoi(argv[2]);

    int ret = SetGpio(gpio, io, val);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

static int GpioModeHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 3, RET_INVALID_PARAM);

    int gpio = atoi(argv[0]);
    int io = atoi(argv[1]);
    int mode = atoi(argv[2]);

    int ret = SetGpioMode(gpio, io, mode);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

static int WifiConnHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 2, RET_INVALID_PARAM);

    char cmd[256] = {0};

    sprintf(cmd,
            "sed '6c ssid=\"%s\"' /system/etc/firmware/wpa_supplicant.conf",
            argv[0]);

    int ret = system(cmd);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    sprintf(cmd, "sed '9c psk=\"%s\"' /system/etc/firmware/wpa_supplicant.conf",
            argv[1]);

    ret = system(cmd);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return RET_OK;
}

static int I2cTestHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 1, RET_INVALID_PARAM);

    uint8_t buf[1] = {0};
    int ret = I2cRead(argv[0], 0x53, 0x05, buf, 1);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    LOG_I("===%u\n", buf[0]);

    return RET_OK;
}

static int EthPhyTestHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 1, RET_INVALID_PARAM);

    int mode = atoi(argv[0]);
    CHECK_RETURN(mode == 1 || mode == 2 || mode == 4, RET_INVALID_PARAM);

    uint16_t valIn = 0;

    switch (mode) {
    case 1:
        valIn = 0x2000;
        break;
    case 2:
        valIn = 0x4000;
        break;
    case 4:
        valIn = 0x8000;
        break;
    }

    int ret = PhyTest("eth1", 1, 0x1F, 0x0000);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    ret = PhyTest("eth1", 1, 0x09, valIn);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return ret;
}

static int UsbTestHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 1, RET_INVALID_PARAM);

    int ret = system("sh /home/root/USB/toUsbHost.sh");
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    ret = system("/unit_tests/memtool 0x38100424=0x40000000");

    return ret;
}

static int LcdTestHandle(int argc, char *argv[], SendToPc toPc)
{
    CHECK_RETURN(argc == 2, RET_INVALID_PARAM);

    int ret = RET_ERROR;

    if (strcmp(argv[0], "PIC") == 0) {
        int index = atoi(argv[1]);
        ret = LcdShowBmp(index);
    } else if (strcmp(argv[0], "COLOR") == 0) {
        int color = strtol(argv[1], NULL, 16);
        ret = LcdFillColor(color);
    }

    return ret;
}

static int HallTestHandle(int argc, char *argv[], SendToPc toPc)
{
    // CHECK_RETURN(argc == 2, RET_INVALID_PARAM);

    return RET_OK;
}

static int LightSensorTestHandle(int argc, char *argv[], SendToPc toPc)
{
    // CHECK_RETURN(argc == 2, RET_INVALID_PARAM);

    return RET_OK;
}

static AtCmd g_cmds[] = {
    {AT_GPIO_READ, GpioReadHandle},
    {AT_GPIO_WRITE, GpioWriteHandle},
    {AT_GPIO_MODE, GpioModeHandle},
    {AT_WIFI_CONN, WifiConnHandle},
    {AT_I2C_TEST, I2cTestHandle},
    {AT_ETH_PHY_TEST, EthPhyTestHandle},
    {AT_USB_TEST, UsbTestHandle},
    {AT_LCD_TEST, LcdTestHandle},
    {AT_HALL_TEST, HallTestHandle},
    {AT_LIGHT_SENSOR_TEST, LightSensorTestHandle},
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