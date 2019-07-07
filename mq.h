/************************************************************************************************
* File name   : mq.h                                                                            *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The function declaration used for mqueue, enums and structures.                 *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include <mqueue.h>
#include <stdbool.h>

/* Message queues for all the tasks */
#define MAINQUEUENAME   "/mainqueue"
#define TEMPQUEUENAME   "/tempqueue"
#define LIGHTQUEUENAME  "/lightqueue"
#define SOCKETQUEUENAME "/socketqueue"
#define LOGQUEUENAME    "/logqueue"

/* Message queue size for all the tasks */
#define MAIN_QUEUE_SIZE   10
#define TEMP_QUEUE_SIZE   5
#define LIGHT_QUEUE_SIZE  5
#define SOCKET_QUEUE_SIZE 5
#define LOG_QUEUE_SIZE    10

typedef enum {
	request,
	update
}messageTypeEnum;

typedef enum{
	fail,
	success,
	init_success,
	init_failure,
}statusEnum;

typedef enum{
	debug,
	alert,
	info
}logLevelEnum;

/* SStructure to communicate to the maintask*/
typedef struct
{
    char source[20];
	messageTypeEnum messageType;
	statusEnum status;
	char unit[20];
}mainStruct;

/* Structure to communicate to the temperature task*/
typedef struct{
	char source[20];
	char unit[20];
}tempStruct;


/* Structure to communicate to the light task*/
typedef struct{
	char source[20];
}lightStruct;

/* Structure to communicate to the logger task*/
typedef struct{
	logLevelEnum logLevel;
	char* source;
	statusEnum status;
	char* message;	
	float value;
	char unit[20];
}logStruct;

/* Structure to communicate to the socket task*/
typedef struct{
	char* source;
	float value;
	char* unit;
	char message[30];
}socketStruct;

/*user defined functions*/

/**
 * @name: mqueue_init
 * 
 * @param1: message queue name
 * @param2: max message queue size
 * @param3: size of the data to send
 * 
 * @description: wrapper around mq_open function. Sets the attributes of the queue and opens the queue with the specified parameters.
 * 
 * return: message queue file descriptor.
 * */
mqd_t mqueue_init(const char*, int, int);
