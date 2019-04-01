/************************************************************************************************
* File name   : logger.c                                                                        *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function declaration used for communication by logger task.                 *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mq.h"

/**
 * @name: logToFile
 * 
 * @desc: Takes the structure with the data as a parameter. Prints this data to the file along with the timestamp, source of the message, loglevel,
 * value and unit. 
 * The source can be lightsensor,tempsensor, maintask and sockettask. 
 * Three log levels are defined. 
 * 1)DEBUG: to print general debug information
 * 2)INFO: printing the temp and light sensor values
 * 3)ALERT: important error messages.
 * 
 * @param1: char*: log file name
 * @param2: logStruct: the populated logger structure that needs to be printed to the file
 * 
 * */
void logToFile(char *, logStruct);

/**
 * name: printTimeStamp
 * 
 * @desc: takes the present timestamp using clock_gettime function, converts it into a string and returns that string.
 * 
 * @return : (char*)- timestamp string.
 * */
char *printTimeStamp();
