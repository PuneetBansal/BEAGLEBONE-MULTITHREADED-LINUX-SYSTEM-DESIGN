/************************************************************************************************
* File name   : temp_i2c.c                                                                      *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for reading and configuring temperature sensor TMP102 for    *
*               getting the temperature values through i2c interface.                           *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/

/************ The standard C libraries included for functionality ************/
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>

/************* The user library containing required information *************/
#include "temp_i2c.h"
#include "myi2c.h"
//#include "logger.h"


int temp_i2c_init(uint8_t slave_addr)
{
	return myi2cInit(slave_addr);
}


int temp_i2c_write_to_reg(int file_des, uint8_t temp_sens_reg_to_write, int16_t data_to_write)
{
	uint8_t buffer[3];
	buffer[0] = temp_sens_reg_to_write;
	uint16_t config_reg, config_value;

	switch(temp_sens_reg_to_write)
	{
	// If there is write to temperature read only register
	case TEMP_REG_ADDR:
		printf("%s::Not allowed to write to read only register\n",__func__);
	break;

	// If there is write to config register
	case CONFIG_REG_ADDR:
		config_value = temp_i2c_read_from_reg(file_des, CONFIG_REG_ADDR);
		config_reg = data_to_write;
		buffer[1] = config_reg >> 8;
		buffer[2] = config_reg;
	break;

	// If there is write operation to Tlow or Thigh register
	case TLOW_REG_ADDR:
	case THIGH_REG_ADDR:
		data_to_write = data_to_write/0.0625;
		buffer[1] = data_to_write >> 4;
		buffer[2] = data_to_write << 4;
	break;
	}
	// Write the buffer values to temp sensor register using i2c
	if(myi2cWrite(file_des, buffer, sizeof(buffer)) < 0)
	{
		printf("%s\n",__func__);
		perror("Write failed: ");
        return -1;
	}
	return 0;
}


uint16_t temp_i2c_read_from_reg(int file_des, uint8_t temp_sens_to_read_from)
{
	uint8_t* buffer;
	uint16_t reg_val;
	if(myi2cWrite(file_des, &temp_sens_to_read_from, sizeof(temp_sens_to_read_from)) < 0)
	{
		printf("%s\n",__func__);
		perror("Write failed: ");
		return 10000;
	}
	buffer = myi2cRead(file_des, 2);
    if(buffer == NULL)
    {
        printf("Temperature read failed\n");
        return 10000;
    }
	//printf("Value of buffer in %s is %x-%x\n",__func__,buffer[0],buffer[1]);
	return ((uint16_t)buffer[0] << 8 | buffer[1]);
}


float read_temperature(int file_des, uint8_t temp_sens_to_read_from)
{
	int16_t temperature;
	float final_temp;

	if(temp_sens_to_read_from == CONFIG_REG_ADDR)
	{
		printf("%s::Config register values are not temperature\n",__func__);
		exit(1);
	}
	temperature = temp_i2c_read_from_reg(file_des, temp_sens_to_read_from);
    if(temperature == 10000)
    {
            return 10000;
    }
	//printf("temperature = %x\n",temperature);
	temperature = temperature >> 4;
	//printf("temperature = %x\n",temperature);
	final_temp = temperature * 0.0625;
	//printf("final_temp = %d\n",final_temp);
	return final_temp;
}

float convert_to_unit(float value, int temperature_unit)
{
	float final_temp;
	switch(temperature_unit)
	{
	case Celsius:
		final_temp = value;
		break;
	case Fahrenheit:
		final_temp = value * 1.8;
		final_temp = final_temp + 32;
		break;
	case Kelvin:
		final_temp = value + 273.5;
		break;
	}
	return final_temp;
}
