#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "tonly_NB.h"

#define GET_GPS_DATA_TIMER           APP_TIMER_TICKS(3000)

APP_TIMER_DEF(m_GetGPSrData_timer_id);

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */


static char       gps_data[150]={0};    /**< RX buffer.   */

static char longitude[15]; //经度 有效数据前10位
static char latitude[15];  //纬度 有效数据前11位
char send_gps_data[22];
char NB_gsp_data[80] = {0};
#if 1
// $GNGGA,122958.000,2302.19655,N,11420.01337,E,1,10,0.84,33.2,M,-1.9,M,,*50<CR><LF>

int gps_analyse()
{
    memset(longitude,0,sizeof(longitude));
    memset(latitude,0,sizeof(latitude));
    char flag_type = ',';
    char *p,*q;
    p = strtok(gps_data, ",");
    if(strncmp(longitude,"$GNGGA",6))
    {
        p = strtok(NULL,",");
        p = strtok(NULL,","); //获取到纬度信息
        
        if(strlen(p) != 10)
        {
            //error data
            return -1;
        }

        q = strtok(NULL,",");
        q = strtok(NULL,","); //获取到经度信息
        if(strlen(q) != 11)
        {
            //error data
            return -1;
        }
        snprintf(send_gps_data, sizeof(send_gps_data),"%s%s",p,q);
        snprintf(NB_gsp_data,sizeof(NB_gsp_data),"AT+NMGS=24,0016%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\r\n",send_gps_data[0],send_gps_data[1],\
            send_gps_data[2],send_gps_data[3],send_gps_data[4],send_gps_data[5],send_gps_data[6],send_gps_data[7],send_gps_data[8],send_gps_data[9],flag_type,\
            send_gps_data[10],send_gps_data[11],send_gps_data[12],send_gps_data[13],send_gps_data[14],send_gps_data[15],send_gps_data[16],send_gps_data[17],\
            send_gps_data[18],send_gps_data[19],send_gps_data[20]);
    }
    return 0;
}
#endif



void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

bool get_spi_flag()
{
    return spi_xfer_done;
}



void spi_init()
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
	spi_config.mode  = NRF_DRV_SPI_MODE_1;
    nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
}




void get_gps_data()
{
	
    spi_xfer_done = false;
    nrf_drv_spi_transfer(&spi,NULL,sizeof(NULL), gps_data, sizeof(gps_data));
    while(spi_xfer_done == false);
	
}

static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    
    //err_code = app_timer_create(&m_GetGPSrData_timer_id, APP_TIMER_MODE_SINGLE_SHOT, get_gps_data);
    APP_ERROR_CHECK(err_code);
    
}

void GPS_init()
{
	
	//gps on
	nrf_gpio_cfg_output(GPS_ON_OFF);
	nrf_gpio_pin_write(GPS_ON_OFF,1);
		
	spi_init();
			
	timers_init();
	
	//Run timer to get GPS data
	//app_timer_start(m_GetGPSrData_timer_id, GET_GPS_DATA_TIMER, NULL);
	
}

