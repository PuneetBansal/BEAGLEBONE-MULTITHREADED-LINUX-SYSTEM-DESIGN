/************************************************************************************************
* File name   : logger.c                                                                        *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The function prototypes, structures, enums and macros used for initialising the *
*               file and writing data to file. The message queue is setup for getting the       *
*               required readings from tasks and writing it to a log file.                      *
* Tools used  : GNU make.                                                                       *
************************************************************************************************/
#include <stdint.h>
#include <stdio.h>

/* Format of message that is sent to logger thread by temperature, light or main thread */
typedef struct{
	char* source;
	int message_type;
	char* message;	
	float value;
	int unit;
}logger_msg;

/* Enumeration for type of message whether success or failure */
enum message_type{
	success,
	fail,
};

/* Macros for queue setup */
#define LOGGER_QUEUE       "/logging"
#define LOGGER_QUEUE_SIZE  15

/* The functions that are used to log the messages to the log file */

/* 
* Function name:- logfile_init
* Description:- This function opens the file for logging the data and returns the file descriptor 
*               used for logging the file.
* @param:- char* (File name for logging the messages)
* @return:- FILE* (file descriptor)
*/
FILE* logfile_init(char*);


/* 
* Function name:- logfile_write
* Description:- This function takes the file descriptor and logger message structure as argument. 
*               Based on the source of the message the function logs the appropriate strings to
*               the file. 
* @param:- FILE* (File descriptor), logger_msg(structure containing logger messages)
* @return:- void
*/
void logfile_write(FILE*, logger_msg);


/* 
* Function name:- timestamp
* Description:- This function returns the fimestamp in formatted string. 
* @param:- void
* @return:- char* (formatted timestamp);
*/
char* timestamp()
