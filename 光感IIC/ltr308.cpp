#include "ltr308.h"
#include "assert_check.h"
#include "error_code.h"
#include "i2c.h"
#include "log.h"
#include "ustimer.h"

#define LTR308_WINDOW_FACTOR   1
#define DEFAULT_COEFFICIENT    (0.6)
#define RETRY_COUNT            (5)

#define LTR308_SLAVE_ADDR      0x53 /* LTR308 7 bit address */
#define LTR308_MAIN_CTRL       0x00 /* ALS operation mode control, SW reset */
#define LTR308_ALS_MEAS_RATE   0x04 /* ALS measurement rate and resolution in Active Mode */
#define LTR308_ALS_GAIN        0x05 /* ALS analog Gain*/
#define LTR308_PART_ID         0x06 /* Part number ID and revision ID */
#define LTR308_MAIN_STATUS     0x07 /* Power-On status, Interrupt status, Data status */
#define LTR308_ALS_DATA_0      0x0D /* ALS ADC measurement ltr308, LSB */
#define LTR308_ALS_DATA_1      0x0E /* ALS ADC measurement ltr308 */
#define LTR308_ALS_DATA_2      0x0F /* ALS ADC measurement ltr308, MSB */
#define LTR308_INT_CFG         0x19 /* Interrupt configuration */
#define LTR308_INT_PST         0x1A /* Interrupt persist setting */
#define LTR308_ALS_THRES_UP_0  0x21 /* ALS interrupt upper threshold, LSB */
#define LTR308_ALS_THRES_UP_1  0x22 /* ALS interrupt upper threshold, intervening bits */
#define LTR308_ALS_THRES_UP_2  0x23 /* ALS interrupt upper threshold, MSB */
#define LTR308_ALS_THRES_LOW_0 0x24 /* ALS interrupt lower threshold, LSB */
#define LTR308_ALS_THRES_LOW_1 0x25 /* ALS interrupt lower threshold, intervening bits */
#define LTR308_ALS_THRES_LOW_2 0x26 /* ALS interrupt lower threshold, MSB */

typedef struct {
    uint8_t partNumber;
    uint8_t revision;
} Ltr308Info_t;

typedef struct {
    uint8_t powerOnStatus;
    uint8_t alsInterruptStatus;
    uint8_t alsDataStatus;
} Ltr308MainStatus_t;

typedef struct {
    uint8_t alsResolution;
    uint8_t alsMeasurementRate;
} Ltr308MeasRate_t;

static uint8_t g_initDone = 0;

static int ReadLtr308RegValue(uint8_t regAddr, uint8_t *value)
{
    int ret = I2CTransfer(I2C0, LTR308_SLAVE_ADDR, I2C_FLAG_WRITE_READ, &regAddr, sizeof(regAddr), value, sizeof(uint8_t));
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return ret;
}

static int WriteLtr308RegValue(uint8_t regAddr, uint8_t value)
{
    uint8_t seq[2] = { 0 };
    seq[0] = regAddr;
    seq[1] = value;

    int ret = I2CTransfer(I2C0, LTR308_SLAVE_ADDR, I2C_FLAG_WRITE, seq, sizeof(seq) / sizeof(uint8_t), NULL, 0);
    CHECK_RETURN(ret == RET_OK, RET_ERROR);

    return ret;
}

static int Ltr308Enable(void)
{
    int ret;
    uint8_t alsEnable = 2;

    ret = WriteLtr308RegValue(LTR308_MAIN_CTRL, alsEnable);
    USTIMER_Delay(500 * 1000);
    return ret;
}

static int Ltr308Disable(void)
{
    int ret;
    uint8_t alsDisable = 0;

    ret = WriteLtr308RegValue(LTR308_MAIN_CTRL, alsDisable);
    return ret;
}

static int GetLtr308MainStatus(Ltr308MainStatus_t *status)
{
    int ret;
    uint8_t value = 0;

    ret = ReadLtr308RegValue(LTR308_MAIN_STATUS, &value);
    if (ret == 0) {
        status->powerOnStatus = (value >> 5) & 0x01;
        status->alsInterruptStatus = (value >> 4) & 0x01;
        status->alsDataStatus = (value >> 3) & 0x01;
    }
    return ret;
}

static int GetLtr308Info(Ltr308Info_t *sensorInfo)
{
    int ret;
    uint8_t value = 0;

    ret = ReadLtr308RegValue(LTR308_PART_ID, &value);
    if (ret == 0) {
        sensorInfo->partNumber = (value >> 4) & 0x0F;
        sensorInfo->revision = value & 0x0F;
    }
    return ret;
}

static int SetLtr308GainRange(uint8_t index)
{
    int ret;

    ret = WriteLtr308RegValue(LTR308_ALS_GAIN, index);
    if (ret != 0) {
        LOG_E("regAddr 0x%x index: 0x%x failed", LTR308_ALS_GAIN, index);
    }

    return ret;
}

static int SetLtr308MeasRate(Ltr308MeasRate_t measRate)
{
    int ret;
    uint8_t value = 0;

    value = (measRate.alsResolution << 4) | measRate.alsMeasurementRate;
    ret = WriteLtr308RegValue(LTR308_ALS_MEAS_RATE, value);
    if (ret != 0) {
        LOG_E("regAddr 0x%x value: 0x%x failed", LTR308_ALS_GAIN, value);
    }
    return ret;
}

static int ConvertResolutionIndexToItTime(uint8_t alsResolution)
{
    int value;

    switch (alsResolution) {
    case 0: {
        value = 400;
        break;
    }

    case 1: {
        value = 200;
        break;
    }

    case 2: {
        value = 100;
        break;
    }

    case 3: {
        value = 50;
        break;
    }

    case 4: {
        value = 25;
        break;
    }

    default:
        value = 100;
        break;
    }
    return value;
}

static int GetLtr308MeasRate(Ltr308MeasRate_t *measRate)
{
    int ret;
    uint8_t value = 0;

    ret = ReadLtr308RegValue(LTR308_ALS_MEAS_RATE, &value);
    if (ret == 0) {
        measRate->alsResolution = (value >> 4) & 0x07;
        measRate->alsMeasurementRate = value & 0x07;
    }
    return ret;
}

static int GetLtr308GainRange(uint8_t *range)
{
    int ret;
    uint8_t value = 0;

    ret = ReadLtr308RegValue(LTR308_ALS_GAIN, &value);
    if (ret == 0) {
        value &= 0x07;
        switch (value) {
        case 0: {
            *range = 1;
            break;
        }

        case 1: {
            *range = 3;
            break;
        }

        case 2: {
            *range = 6;
            break;
        }

        case 3: {
            *range = 9;
            break;
        }

        case 4: {
            *range = 18;
            break;
        }

        default:
            break;
        }
    }
    return ret;
}

static int GetLtr308AlsRawData(unsigned int *alsData)
{
    int ret;
    uint8_t low = 0;
    uint8_t middle = 0;
    uint8_t high = 0;

    ret = ReadLtr308RegValue(LTR308_ALS_DATA_0, &low);
    if (ret != 0) {
        return ret;
    }

    ret = ReadLtr308RegValue(LTR308_ALS_DATA_1, &middle);
    if (ret != 0) {
        return ret;
    }

    ret = ReadLtr308RegValue(LTR308_ALS_DATA_2, &high);
    if (ret != 0) {
        return ret;
    }

    *alsData = (high << 16) | (middle << 8) | low;

    return ret;
}

void AlsInit(void)
{
    Ltr308Info_t ltrInfo;
    Ltr308MeasRate_t measRate;

    GetLtr308Info(&ltrInfo);
    SetLtr308GainRange(4);
    measRate.alsResolution = 0;
    measRate.alsMeasurementRate = 3;
    SetLtr308MeasRate(measRate);
    g_initDone = 1;

    Ltr308Enable();
}

void AlsEnable(void)
{
    Ltr308Enable();
}

void AlsDisable(void)
{
    Ltr308Disable();
}

int AlsGetValue(float *lux)
{
    int ret = -1;
    uint8_t i = 0;
    uint8_t gain = 0;
    unsigned int luxRawData = 0;
    int itTimeMs = 0;
    Ltr308MainStatus_t status;
    Ltr308MeasRate_t measRate;

    if (g_initDone == 0) {
        LOG_E("please init ALS frist!");
        return -1;
    }

    for (i = 0; i < RETRY_COUNT; i++) {
        ret = GetLtr308MainStatus(&status);
        if (ret != 0) {
            LOG_E("GetLtr308MainStatus failed");
            return ret;
        }

        if (status.alsDataStatus == 1) {
            break;
        }
        USTIMER_Delay(200 * 1000);
    }

    ret = GetLtr308AlsRawData(&luxRawData);
    if (ret != 0) {
        LOG_E("GetLtr308AlsRawData failed");
        return ret;
    }

    ret = GetLtr308GainRange(&gain);
    if (ret != 0) {
        LOG_E("GetLtr308GainRange failed");
        return ret;
    }

    ret = GetLtr308MeasRate(&measRate);
    if (ret != 0) {
        LOG_E("GetLtr308MeasRate failed");
        return ret;
    }

    itTimeMs = ConvertResolutionIndexToItTime(measRate.alsResolution);
    *lux = LTR308_WINDOW_FACTOR * (DEFAULT_COEFFICIENT * luxRawData / (gain * itTimeMs) * 100);

    return ret;
}