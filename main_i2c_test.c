#include <stdio.h>
#include "gpio.h"
#include "temp_i2c.h"

int main()
{
	int i2c_file_d;                             //File descriptor for i2c file
	float temperature;
	uint16_t config;
	i2c_file_d = temp_i2c_init(DEFAULT_SLAVE_ADDRESS);
	printf("The i2c file descriptor from temp is %d\n",i2c_file_d);
	temp_i2c_write_to_reg(i2c_file_d, TLOW_REG_ADDR, 26);
	temp_i2c_write_to_reg(i2c_file_d, THIGH_REG_ADDR, 30);
	temperature = read_temperature(i2c_file_d, TEMP_REG_ADDR, 0);	
	printf("Temperature is %f\n",temperature);
	config = temp_i2c_read_from_reg(i2c_file_d, CONFIG_REG_ADDR);
	printf("CONFIG is %x\n",config);
	return 0;
}
