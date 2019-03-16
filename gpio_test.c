#include "../../HW2/Problem3/buildroot/output/build/linux-4796173fc58688055a99a1cef19a839174067220/include/linux/gpio.h"
#include <stdio.h>

int main()
{
	while(1)
	{
		gpio_set_value(53,1);
		for(long i=0; i<1000000; i++);
		gpio_set_value(53,1);
		for(long i=0; i<1000000; i++);
	}
	return 0;
}
