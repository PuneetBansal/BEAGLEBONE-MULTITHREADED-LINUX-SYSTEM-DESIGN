/************************************************************************************************
* File name   : gpio.c                                                                          *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for gpio operations. Setting the direction of pin and        *
* 				the value. This functions are restricted for use of only USER LED pins.         *
* Tools used  : GNU make.                                                                       *
************************************************************************************************/

/* Including standard libraries */
#include <stdio.h>
#include <stdlib.h>

/* Including user libraries */
#include "gpio.h"

/* gpio_pin high and low value. If different valid range is required then define different 
*  values with the same define names */
#define gpio_low 53
#define gpio_high 56


void gpio_init(int gpio_pin,int gpio_direction)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	if(gpio_pin >= gpio_low && gpio_pin <= gpio_high)
	{
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
	}
	else
	{
		printf("Enter the pin numbers in range %d - %d",gpio_low,gpio_high);
	}
		free(file);
}


void gpio_write_value(int gpio_pin, int gpio_value)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	if(gpio_pin >= gpio_low && gpio_pin <= gpio_high)
	{
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
	}
	else
	{
		printf("Enter the pin numbers in range %d - %d",gpio_low,gpio_high);
	}
	free(file);
}


