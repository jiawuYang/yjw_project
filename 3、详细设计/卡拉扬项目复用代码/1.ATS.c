/*********************************************************************
 * @fn      SimplePeripheral_processCharValueChangeEvt
 *
 * @brief   Process a pending Simple Profile characteristic value change
 *          event.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimplePeripheral_processATSCmdEvt(uint8_t paramID)
{
    uint8_t pdata[12];
    uint8_t notify[12] = {0};
    uint8_t device_sn[8] = {0};
    int ret = 0;

    if (paramID == SIMPLEATS_CHAR1)
    {
        SimpleATS_GetParameter(SIMPLEATS_CHAR1, pdata);
        Display_print4(dispHandle, SBP_ROW_STATUS2, 0, "ATS CMD: %d %d %d, Len: %d", pdata[0], pdata[1], pdata[2], CurATSCharLength);
        if (SIMPLEATS_NORMAL_LEN == CurATSCharLength)
        {
            notify[0] = pdata[0];
            notify[1] = pdata[1];
            switch ( pdata[0] )
            {
                case 0x90:
                    switch ( pdata[1] )
                    {
                        case 0x00:
                            Display_print0(dispHandle, SBP_ROW_STATUS2, 0, "Enter factory mode");

                            Util_stopClock(&getSensorValueClock);
                            Util_stopClock(&getTHValueClock);
                            Util_stopClock(&checkPowerSupplyClock);
                            Util_stopClock(&lightMeasureClock);
                            Util_stopClock(&disConnectAPPClock);

                            EnableBatteryAdc();
                            EnableCapacitanceAdc();
                            ALS_Init();
                            ALS_Enable();

                            notify[2] = 0x10;
                            notify[3] = 0x00;
                            SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);

                            break;
                        case 0x01:
                            Display_print0(dispHandle, SBP_ROW_STATUS2, 0, "Exit factory mode");
                            device_property.ats_flag = 0xBB;
                            DeviceProperty_writeToFlash();
                            DeviceProperty_readFromFlash();
                            if (device_property.ats_flag == 0xBB)
                            {
                                notify[2] = 0x10;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }

                            break;
                        case 0x02:
                            //ver
                            uint8_t version[4] = SOFTWARE_VER;
                            notify[2] = 0x10;
                            notify[3] = 0x04;
                            VOID memcpy(&notify[4], version, 4);
                            SimpleATS_SetParameter(SIMPLEATS_CHAR1, 8, notify);

                            break;
                        case 0x03:
                            //mac
                            uint8_t ownAddress[B_ADDR_LEN];
                            bStatus_t ret = SUCCESS;
                            ret = GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);
                            if (ret == SUCCESS)
                            {
                                notify[2] = 0x10;
                                notify[3] = 0x06;
                                //VOID memcpy(&notify[4], ownAddress, B_ADDR_LEN);
                                notify[4] = ownAddress[5];
                                notify[5] = ownAddress[4];
                                notify[6] = ownAddress[3];
                                notify[7] = ownAddress[2];
                                notify[8] = ownAddress[1];
                                notify[9] = ownAddress[0];

                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 10, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }


                            break;
                        default:
                            break;
                    }
                    break;
                case 0x50:
                    switch ( pdata[1] )
                    {
                        case 0x01:
                            //T
                            getTHValue();

                            if (sensor_value.error.sht3x_error == 1)
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);

                            }
                            else
                            {
                                int16_t temperature = get_temperature_value();
                                notify[2] = 0x10;
                                notify[3] = 0x02;
                                notify[4] = LO_UINT16(temperature);
                                notify[5] = HI_UINT16(temperature);
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 6, notify);
                            }

                            break;
                        case 0x02:
                            //H
                            getTHValue();

                            if (sensor_value.error.sht3x_error == 1)
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);

                            }
                            else
                            {
                                uint16_t humidity = get_humidity_value();
                                notify[2] = 0x10;
                                notify[3] = 0x02;
//                                notify[4] = get_humidity_value();
                                notify[4] = LO_UINT16(humidity);
                                notify[5] = HI_UINT16(humidity);
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 6, notify);
                            }

                            break;
                        case 0x03:
                            //L
                            float lux_value = 0;

                            if (ALS_GetValue(&lux_value) == 0)
                            {
                                sensor_value.lux_val = (unsigned short)(lux_value);
                                notify[2] = 0x10;
                                notify[3] = 0x02;
                                notify[4] = LO_UINT16(sensor_value.lux_val);
                                notify[5] = HI_UINT16(sensor_value.lux_val);
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 6, notify);
                                Display_print1(dispHandle, 12, 0, "lux_val: %d", sensor_value.lux_val);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }

                            break;
                        case 0x04:
                            //extern battery
                            uint32_t batterVoltage = 0;

                            ret = ReadVoltageFromAdc(Board_ADC0, &batterVoltage);
                            if (ret == 0)
                            {
                                CalculateVoltage(&batterVoltage);
                                batterVoltage /= 1000;

                                notify[2] = 0x10;
                                notify[3] = 0x02;
                                notify[4] = ((batterVoltage) & 0xFF);
                                notify[5] = ((batterVoltage >> 8) & 0xFF);
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 6, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }

                            break;
                        case 0x05:
                            //cap battery
                            uint32_t capVoltage = 0;

                            ret = ReadVoltageFromAdc(Board_ADC2, &capVoltage);
                            if (ret == 0)
                            {
                                CalculateVoltage(&capVoltage);
                                capVoltage /= 1000;

                                notify[2] = 0x10;
                                notify[3] = 0x02;
                                notify[4] = ((capVoltage) & 0xFF);
                                notify[5] = ((capVoltage >> 8) & 0xFF);
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 6, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }

                            break;

                        case 0x06:
                            DeviceData_readFromFlash(&notify[4]);

                            notify[2] = 0x10;
                            notify[3] = 0x08;
                            SimpleATS_SetParameter(SIMPLEATS_CHAR1, 12, notify);
                            break;

                        case 0x00:
                            //date
                            DeviceProperty_readFromFlash();

                            notify[2] = 0x10;
                            notify[3] = 0x03;
                            notify[4] = device_property.WP_sensor_ID[0];
                            notify[5] = device_property.WP_sensor_ID[1];
                            notify[6] = device_property.WP_sensor_ID[2];
                            SimpleATS_SetParameter(SIMPLEATS_CHAR1,7, notify);

                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            notify[0] = pdata[0];
            notify[1] = pdata[1];
            switch ( pdata[0] )
            {
                case 0x80:
                    switch ( pdata[1] )
                    {
                        notify[0] = pdata[0];
                        notify[1] = pdata[1];
                        case 0x00:
                            device_property.WP_sensor_ID[0] = pdata[3];
                            device_property.WP_sensor_ID[1] = pdata[4];
                            device_property.WP_sensor_ID[2] = pdata[5];

                            uint8_t status = DeviceProperty_writeToFlash();
                            if (status == SUCCESS)
                            {
                                notify[2] = 0x10;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1,4, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }

                            break;

                        case 0x06:
                            int i;
                            for(i=3;i<10;i++)
                            {
                                if((pdata[i]<0x30) || (pdata[i]>0x39))
                                {
                                    notify[2] = 0x00;
                                    notify[3] = 0x00;
                                    SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                                    return;
                                }
                            }

                            device_sn[0] = pdata[3];
                            device_sn[1] = pdata[4];
                            device_sn[2] = pdata[5];
                            device_sn[3] = pdata[6];
                            device_sn[4] = pdata[7];
                            device_sn[5] = pdata[8];
                            device_sn[6] = pdata[9];
                            device_sn[7] = pdata[10];

                            uint8_t devicedata_status = DeviceData_writeToFlash(device_sn);
                            if (devicedata_status == SUCCESS)
                            {
                                notify[2] = 0x10;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1,4, notify);
                            }
                            else
                            {
                                notify[2] = 0x00;
                                notify[3] = 0x00;
                                SimpleATS_SetParameter(SIMPLEATS_CHAR1, 4, notify);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

        }

    }
}
