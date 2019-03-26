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

/* Message queue structure to send to the tasks */
typedef struct
{
    char source[20];
	messageTypeEnum messageType;
	statusEnum status;
	char unit[20];
}mainStruct;

typedef struct{

}tempStruct;

typedef struct{

}lightStruct;


typedef struct{
char* source;
statusEnum status;
char* message;	
float value;
char unit[20];
}logStruct;

typedef struct{

}socketStruct;

/*user defined functions*/

mqd_t mqueue_init(const char*, int, int);
