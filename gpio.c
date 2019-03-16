#include <stdio.h>
#include <stdlib.h>
#include "gpio.h"


void gpio_init(int gpio_pin,int gpio_direction)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	fp = fopen("/sys/class/gpio/export","w");
	fprintf(fp,"%d",gpio_pin);
	fclose(fp);
	sprintf(file,"/sys/class/gpio/gpio%d/direction",gpio_pin);
	fp = fopen(file,"w");
	if(gpio_direction == out)
	{
		fprintf(fp,"out");
	}
	else if(gpio_direction == in)
	{
		fprintf(fp,"in");
	}
	else
	{
		printf("Enter direction only as in or out");
	}
	fclose(fp);
	free(file);
}


void gpio_write_value(int gpio_pin, int gpio_value)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	sprintf(file,"/sys/class/gpio/gpio%d/value",gpio_pin);
	fp = fopen(file,"w");
	if(gpio_value == low)
	{
		fprintf(fp,"%d",low);
	}
	else if(gpio_value == high)
	{
		fprintf(fp,"%d",high);
	}
	else
	{
		printf("Enter value only as low or high");
	}
	fclose(fp);
	free(file);
}


