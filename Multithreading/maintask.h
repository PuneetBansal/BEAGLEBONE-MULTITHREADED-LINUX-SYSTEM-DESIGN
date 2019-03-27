//#include "mq.h"
#include "logger.h"

#define TEMPERATURE_TASK 0x01
#define LIGHT_TASK       0x02
#define LOGGER_TASK      0x04
#define SOCKET_TASK      0x08

typedef struct 
{
  int data; 
}mainInfoToOthers;
