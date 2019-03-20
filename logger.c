/************************************************************************************************
* File name   : logger.c                                                                        *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for initialising the file and writing data to file. The      *
*               message queue is setup for getting the required readings from tasks and writing *
*               it to a log file.                                                               *
* Tools used  : GNU make.                                                                       *
************************************************************************************************/

/************ The standard C libraries included for functionality ************/
#include <stdio.h>
#include <time.h>

/************* The user library containing required information *************/
#include "logger.h"
#include "temp_i2c.h"


/* Function prototypes */
FILE* logfile_init(char* log_file)
{
	FILE *logging;
	logging = fopen(log_file,"a");
	return log_file;	
}


void logfile_write(FILE* log_file, logger_msg log_struct)
{
	if((strcmp(log_struct.source,"temperature"))==0)
	{
		if(log_struct.message_type == success)
		{
			fprintf(log_file, "[%s] %f",timestamp(), log_struct.value);
			switch(log_struct.unit)
			{
			case Celsius:
				fprintf(log_file, " Celsius\n");
				break;
			case Fahrenheit:
				fprintf(log_file, " Fahrenheit\n");
				break;
			case Kelvin:
				fprintf(log_file, " Kelvin\n");
				break;
			}
		}
		else
		{
			fprintf(log_file, "(Failure message) - %s",log_struct.message);
		}
	}
	else if((strcmp(log_struct.source,"light"))==0)
	{
		//See if it a success or failure message and log the file accordingly.
	}
}


//timestamp function referenced from https://stackoverflow.com/questions/3673226/how-to-print-time-in-format-2009-08-10-181754-811
char* timestamp()
{
	time_t timer;	
	char* time_stamp = (char*)malloc(20);
	struct tm *time_struct;

	time(&timer);
	time_struct = localtime(&timer);
	strftime(time_stamp, 20, "%Y-%m-%d %H-%M-%S", time_struct);
	return time_stamp;
}


//Create a logger function to be executed as a logger task
