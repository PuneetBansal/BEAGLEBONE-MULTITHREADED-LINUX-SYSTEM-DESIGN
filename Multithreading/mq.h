#include <mqueue.h>

/* Message queues for all the tasks */
#define MAINQUEUENAME   "/mainqueue"
#define TEMPQUEUENAME   "/tempqueue"
#define LIGHTQUEUENAME  "/lightqueue"
#define SOCKETQUEUENAME "/socketqueue"
#define LOGQUEUENAME    "/logqueue"

/* Message queue size for all the tasks */
#define MAIN_QUEUE_SIZE   20
#define TEMP_QUEUE_SIZE   5
#define LIGHT_QUEUE_SIZE  5
#define SOCKET_QUEUE_SIZE 5
#define LOG_QUEUE_SIZE    10


/* Message queue structure to send to the tasks */
typedef struct
{
    int data;
}mainStruct;

typedef struct{

}tempStruct;

typedef struct{

}lightStruct;

typedef struct{

}logStruct;

typedef struct{

}socketStruct;

/*user defined functions*/

mqd_t mqueue_init(const char*, int, int);