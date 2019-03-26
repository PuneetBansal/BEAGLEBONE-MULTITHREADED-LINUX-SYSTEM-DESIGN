/************************************************************************************************
* File name   : gpio.c                                                                          *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for gpio operations. Setting the direction of pin and        *
*               the value. This functions are restricted for use of only USER LED pins.         *
* Tools used  : GNU make.                                                                       *
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


void enable_watch_on_pin(int gpio_pin, char* handler_name)
{
	FILE *fp;
	char *file = (char*)malloc(40);
	struct sigaction signal_act;
	int fd;
	
	if(is_pin_valid(gpio_pin))
	{
		sprintf(file,"/sys/class/gpio/gpio%d",gpio_pin);
		
		signal_act.sa_sigaction = (void*)handler_name;
		signal_act.sa_flags     = SA_SIGINFO;
		sigaction(SIGRTMIN + 1, &signal_act, NULL);

		if((fd = open(file, O_RDONLY)) < 0)
			perror("Failed to open the file");

		if((fcntl(fd, F_SETSIG, SIGRTMIN + 1)) < 0)
			perror("Setting signal for file change failed");

		if((fcntl(fd, F_NOTIFY, DN_MODIFY)) < 0)
			perror("Notifying file failed");
	}
	else
	{
		printf("Enter valid pin number");
	}
	free(file);
}
