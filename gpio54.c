#include <stdio.h>
#include "gpio.h"

int main()
{
	gpio_init(54,out);
	gpio_write_value(54,high);
	/*while(1)
	{
		fp = fopen("/sys/class/gpio/gpio54/value","w");
		fprintf(fp,"0");
		fclose(fp);
		for(long long i=0; i<99999999; i++);
		fp = fopen("/sys/class/gpio/gpio54/value","w");
		fprintf(fp,"1");
		fclose(fp);
		for(long long i=0; i<199999999; i++);
	}*/
	return 0;
}
