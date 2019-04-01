/************************************************************************************************
* File name   : gpio.c                                                                          *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for gpio operations. Setting the direction of pin and        *
*               the value. This functions are restricted for use of only USER LED pins.         *
* Tools used  : GNU make, gcc, arm-linux-gcc.                                                   *
************************************************************************************************/
#define _GNU_SOURCE

/* Including standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/gpio.h>

/* Including user libraries */
#include "gpio.h"


void gpio_init(int gpio_pin,int gpio_direction)
{
	FILE *fp;
	char *file = (char*)malloc(40);

	if(is_pin_valid(gpio_pin))
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
		printf("Enter valid pin number");
	}
		free(file);
}


void gpio_write_value(int gpio_pin, int gpio_value)
{
	FILE *fp;
	char *file = (char*)malloc(40);

	if(is_pin_valid(gpio_pin))
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
		printf("Enter valid pin number");
	}
	free(file);
}


int gpio_read_value(int gpio_pin)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	int value;
	
	if(is_pin_valid(gpio_pin))
	{
		sprintf(file,"/sys/class/gpio/gpio%d/value",gpio_pin);
		fp = fopen(file,"r");
		fscanf(fp,"%d",&value);
		fclose(fp);
	}
	else
	{
		printf("Enter valid pin number");
	}
	free(file);
	return value;
}


bool is_pin_valid(int gpio_pin)
{
	int gpio_allowed[total_gpio] = access_pin_allowed;
	bool is_valid = false;
	
	for(int i=0; i<total_gpio; i++)
	{
		if(gpio_pin == gpio_allowed[i])
			is_valid = is_valid | true;
		else
			is_valid = is_valid | false;
	}
	return is_valid;
}


void gpio_interrupt_state(int gpio_pin, gpio_interrupt interrupt)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	
	if(is_pin_valid(gpio_pin))
	{	
		gpio_init(gpio_pin,in);
		sprintf(file,"/sys/class/gpio/gpio%d/edge",gpio_pin);
		fp = fopen(file,"w");
		switch(interrupt)
		{
		case rising:
			fprintf(fp,"rising");
			break;
		case falling:
			fprintf(fp,"falling");
			break;
		case both:
			fprintf(fp,"both");
			break;
		case none:
			fprintf(fp,"none");
			break;
		}
		fclose(fp);
	}
	else
	{
		printf("Enter valid pin number");
	}
	free(file);
}


int gpio_open_value(int gpio_pin)
{
	char *file = (char*)malloc(40);
	int fd;
	if(is_pin_valid(gpio_pin))
	{
		sprintf(file,"/sys/class/gpio/gpio%d/value",gpio_pin);
		fd = open(file, O_RDONLY);
	}
	else
	{
		printf("Enter valid pin number");
		fd = -1;
	}
	return fd;
}


int gpio_read_val_with_fd(int fd)
{
	int value;
	read(fd, &value, sizeof(value));
	lseek(fd, 0, SEEK_SET);
	return value & 0x1;
}
