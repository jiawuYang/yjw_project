#ifndef _GPIO_I2C_H
#define _GPIO_I2C_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "le501x.h"
#include "platform.h"
#include "io_config.h"
#include "HAL_def.h"
#include "pin_config.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#define I2C_0 (0)
#define I2C_DEFAULT_DELAY (50)

/***************************i2c***********************/
void gpio_i2c_deinit();
void gpio_i2c_write_byte(u8 num, u8 delay, u8 addr, u8 regadd,u8 wdata);
u8 gpio_i2c_read_byte(u8 num, u8 delay, u8 addr, u8 regadd);
void gpio_i2c_write_data(u8 addr, u8 regadd, u8 *pdata, u8 len);
void gpio_i2c_read_data(u8 addr, u8 regadd, u8 *pdata, u8 len);
/***************************end***********************/


#endif

