#include "gpio_i2c.h"

#define SCL_SET_1                     \
    {                                 \
        io_write_pin(P_I2C_SCL, 1); \
    }
#define SCL_SET_0                     \
    {                                 \
        io_write_pin(P_I2C_SCL, 0); \
    }
#define SDA_SET_1                     \
    {                                 \
        io_write_pin(P_I2C_SDA, 1); \
    }
#define SDA_SET_0                     \
    {                                 \
        io_write_pin(P_I2C_SDA, 0); \
    }
#define SDA_GET io_read_pin(P_I2C_SDA)

#define SET_SCL_OUT()               \
    {                               \
        io_cfg_output(P_I2C_SCL); \
    }
#define SET_SDA_OUT()               \
    {                               \
        io_cfg_output(P_I2C_SDA); \
    }
#define SET_SDA_IN()                            \
    {                                           \
        io_cfg_input(P_I2C_SDA);              \
        io_pull_write(P_I2C_SDA, IO_PULL_UP); \
    }

void gpio_i2c_deinit()
{
    iic1_io_init( P_I2C_SCL, P_I2C_SDA);
    DELAY_US(1000 * 5);
    iic1_io_deinit();
}
//--------------------------------------------------------------
// Prototype : void delay_us(u8 times)
// Description : delay xx us
//--------------------------------------------------------------
static void delay_us(u8 times)
{
    DELAY_US(times);
}

/*
void i2c_test(void)
{
     SCL_SET_1; //SCL = 1;
     DELAY_US(5);
     SCL_SET_0; //SCL = 1;
     DELAY_US(5);

     SCL_SET_1; //SCL = 1;
     DELAY_US(5);
     SCL_SET_0; //SCL = 1;
     DELAY_US(5);
}
*/
//--------------------------------------------------------------
// Prototype : void gpio_i2c_Start(void)
// Calls : delay_us()
// Description : Start Singnal
//--------------------------------------------------------------
static void gpio_i2c_start(u8 delay)
{
    delay_us(delay);

    //SDA 1->0 while SCL High
    SDA_SET_1; //SDA = 1;
    SCL_SET_1; //SCL = 1;
    delay_us(delay);
    SDA_SET_0; //SDA = 0;
    delay_us(delay);
    SCL_SET_0; //SCL = 0;
}

//--------------------------------------------------------------
// Prototype : void gpio_i2c_Stop(void)
// Calls : delay_us()
// Description : Stop Singnal
//--------------------------------------------------------------
static void gpio_i2c_stop(u8 delay)
{
    SCL_SET_0;
    delay_us(delay);
    SDA_SET_0;
    delay_us(delay);

    SCL_SET_1;
    delay_us(delay);
    SDA_SET_1;
    delay_us(delay);
}
//--------------------------------------------------------------
// Prototype : void gpio_i2c_SendACK(u8 ack);
// Calls : delay_us()
// Parameters : bit ack:1-noack, 0-ack
// Description : Master device send ACK to slave device.
//--------------------------------------------------------------
static void gpio_i2c_send_ack(u8 delay, u8 ack)
{
    if (ack == 0)
    {
        SDA_SET_0; //SDA = 0;
    }
    else
    {
        SDA_SET_1; //SDA = 1;
    }

    SCL_SET_1; //SCL = 1;
    delay_us(delay);
    delay_us(delay);
}
//--------------------------------------------------------------
// Prototype : u8 gpio_i2c_SendByte(u8 sendDAT)
// Calls : delay_us()
// Parameters : u8 sendDAT---data to be send
// Return Value : CY--slave ack (1---noack, 0--ack)
// Description : Send one byte to gpio_i2c
//--------------------------------------------------------------
static void gpio_i2c_send_byte(u8 delay, u8 data)
{
    u8 i = 0;
    FlagStatus ack = SET;
    for (i = 0; i < 8; i++)
    {
        SCL_SET_0; //SCL = 0;
        delay_us(delay);
        if (data & 0x80) // write data
        {
            SDA_SET_1; //SDA = 1;
        }
        else
        {
            SDA_SET_0; //SDA = 0;
        }
        data <<= 1;
        delay_us(delay);

        SCL_SET_1; //SCL = 1;
        delay_us(delay);
        delay_us(delay);
    }

    SCL_SET_0; //SCL = 0;
    delay_us(delay);
    SET_SDA_IN();
    delay_us(delay);

    SCL_SET_1; //SCL = 1;

    delay_us(delay);
    ack = SDA_GET;
    if (ack == SET)
    {
        //error
    }
    delay_us(delay);

    SCL_SET_0; //SCL = 0;
    SET_SDA_OUT();

    return;
}

//--------------------------------------------------------------
// Prototype : u8 gpio_i2c_RecvByte()
// Calls : delay_us()
// Parameters : none
// Return Value : revDAT- received data
// Description : Receive one byte from gpio_i2c
//--------------------------------------------------------------
static u8 gpio_i2c_recv_byte(u8 delay)
{
    u8 i;
    u8 data = 0;

	SCL_SET_0; //SCL = 0;
    SDA_SET_1; //SDA = 1; // latch the Data port befor reading
    SET_SDA_IN();
    delay_us(delay);
    delay_us(delay);

    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        SCL_SET_1; //SCL = 1;

        delay_us(delay);

        if (SDA_GET == SET)
        {
            data |= 0x01;
        }
        else
        {
            data &= 0xfe;
        }
        delay_us(delay);
        SCL_SET_0; //SCL = 0;

        delay_us(delay);
        delay_us(delay);
    }

    SET_SDA_OUT();

    return data;
}

//***I2C_0, gsensor
static void i2c_choice(u8 num)
{
    SET_SCL_OUT();
    SET_SDA_OUT();
}

//--------------------------------------------------------------
// Prototype : void Write_PT2314(u8 wdata)
// Calls : gpio_i2c_Start(), gpio_i2c_Stop(),gpio_i2c_SendByte()
// Parameters : RegAddr-- target memory address,
// wrdata--- data to be writing
// Description : Write one byte to target memory
//--------------------------------------------------------------
void gpio_i2c_write_byte(u8 num, u8 delay, u8 addr, u8 regadd, u8 wdata)
{
    i2c_choice(num);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x00); // Device Addr + Write (operation)
    gpio_i2c_send_byte(delay, regadd);
    gpio_i2c_send_byte(delay, wdata);

    gpio_i2c_stop(delay);
}

u8 gpio_i2c_read_byte(u8 num, u8 delay, u8 addr, u8 regadd)
{
    u8 val;

    i2c_choice(num);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x00); // Device Addr + Write (operation)
    gpio_i2c_send_byte(delay, regadd);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x01); // Device Addr + Write (operation)

    delay_us(delay);

    val = gpio_i2c_recv_byte(delay);
    gpio_i2c_send_ack(delay, 1);

    gpio_i2c_stop(delay);

    return val;
}

void gpio_i2c_write_data(u8 addr, u8 regadd, u8 *pdata, u8 len)
{
    u8 j = 0;
    u8 delay = I2C_DEFAULT_DELAY;

    i2c_choice(I2C_0);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x00); // Device Addr + Write (operation)
    gpio_i2c_send_byte(delay, regadd);

    for (j = 0; j < len; j++)
    {
        gpio_i2c_send_byte(delay, *(pdata + j));
    }

    gpio_i2c_stop(delay);
}

void gpio_i2c_read_data(u8 addr, u8 regadd, u8 *pdata, u8 len)
{
    u8 i = 0;
    u8 delay = I2C_DEFAULT_DELAY;

    i2c_choice(I2C_0);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x00); // Device Addr + Write (operation)
    gpio_i2c_send_byte(delay, regadd);

    gpio_i2c_start(delay);
    gpio_i2c_send_byte(delay, addr | 0x01); // Device Addr + Write (operation)

    delay_us(delay);

    for (i=0; i<len; i++)
    {
         *(pdata + i) = gpio_i2c_recv_byte(delay);
        if (i < (len - 1))
        {
            gpio_i2c_send_ack(delay, 0);
        }
    }

    gpio_i2c_send_ack(delay, 1);

    gpio_i2c_stop(delay);
}

static u8 gpio_i2c_lcd_recv_byte(u8 delay)
{
    u8 i;
    u8 data = 0;

	// SCL_SET_0; //SCL = 0;
    SDA_SET_1; //SDA = 1; // latch the Data port befor reading
    SET_SDA_IN();
    delay_us(delay);
    delay_us(delay);

    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        SCL_SET_1; //SCL = 1;

        delay_us(delay);

        if (SDA_GET == SET)
        {
            data |= 0x01;
        }
        else
        {
            data &= 0xfe;
        }
        delay_us(delay);
        SCL_SET_0; //SCL = 0;

        delay_us(delay);
        delay_us(delay);
    }

    SET_SDA_OUT();

    return data;
}

void gpio_i2c_lcd_read_data(u8 addr, u8 regadd, u8 *pdata, u8 len)
{
    u8 i = 0;
    u8 delay = I2C_DEFAULT_DELAY;

    i2c_choice(I2C_0);

    gpio_i2c_start(delay);

    gpio_i2c_send_byte(delay, addr | 0x01); // Device Addr + Write (operation)
    gpio_i2c_send_byte(delay, regadd);

    delay_us(delay);

    for (i=0; i<len; i++)
    {
         *(pdata + i) = gpio_i2c_lcd_recv_byte(delay);
        if (i < (len - 1))
        {
            gpio_i2c_send_ack(delay, 0);
        }
    }

    gpio_i2c_send_ack(delay, 1);

    gpio_i2c_stop(delay);
}