#include <stdio.h>
#include "gpio.h"
#include "temp_i2c.h"

int main()
{
	int i2c_file_d;                             //File descriptor for i2c file
	i2c_file_d = temp_i2c_init(DEFAULT_SLAVE_ADDRESS);
	printf("The i2c file descriptor fro temp is %d",i2c_file_d);
	temp_i2c_write_to_reg(i2c_file_d, TLOW_REG_ADDR, 25);	
	return 0;
}
