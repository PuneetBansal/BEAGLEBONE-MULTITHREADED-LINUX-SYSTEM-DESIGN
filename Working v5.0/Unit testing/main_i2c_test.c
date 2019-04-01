#include <stdio.h>
#include "../temp_i2c.h"
#include "linux/i2c-dev.h"
#include "linux/i2c.h"
#include <errno.h>
#include <unistd.h>
#include <assert.h>


int main()
{
	int i2c_file_d;                             //File descriptor for i2c file
	float temperature;
	uint16_t config;
	int ret;
	i2c_file_d = temp_i2c_init(0x48);
	printf("The i2c file descriptor from temp is %d\n",i2c_file_d);
	ret = temp_i2c_write_to_reg(i2c_file_d, TLOW_REG_ADDR, 26);
    if(ret < 0)
    {
        printf("Write failure\n");
        return -1;
    }
	temperature = read_temperature(i2c_file_d, TLOW_REG_ADDR);
	assert(temperature == 26.000000);
	printf("TLOW register value is %f\n",temperature);
	printf("TLOW read successful\n");
	ret = temp_i2c_write_to_reg(i2c_file_d, THIGH_REG_ADDR, 30);
	if(ret < 0)
    {
        printf("Write failure\n");
        return -1;
    }
	temperature = read_temperature(i2c_file_d, THIGH_REG_ADDR);
	assert(temperature == 30.000000);
	printf("THIGH register value is %f\n",temperature);
	printf("THIGH read successful\n");
	temperature = read_temperature(i2c_file_d, TEMP_REG_ADDR);
	if (temperature == 10000)
    {
        printf("Reading from temperature sensor failed\n");
        return -1;
    }
	printf("Temperature is %f Celsius\n",temperature);
	config = temp_i2c_read_from_reg(i2c_file_d, CONFIG_REG_ADDR);
	printf("CONFIG read is %x\n",config);
	ret = temp_i2c_write_to_reg(i2c_file_d, CONFIG_REG_ADDR, DEFAULT_CONFIG | ALERT_ON_6_FAULT);
	config = temp_i2c_read_from_reg(i2c_file_d, CONFIG_REG_ADDR);
	printf("CONFIG after write is %x\n",config);
	assert(config == DEFAULT_CONFIG | ALERT_ON_6_FAULT);
	printf("Config read successful\n");
	return 0;
}
