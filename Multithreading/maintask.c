#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

/*User defined headers*/
#include "maintask.h"

pthread_t tempSensorTask,lightSensorTask,loggerTask,socketTask;

static void* tempSensorRoutine(void *dataObj)
{
    printf("Entered tempSensorTask\n");
    mainStruct dataToSend;
    dataToSend.data=55;
    mqd_t tempToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
    if(tempToMain < 0)
    {
        perror("Main queue creation failed");
    }
    mq_send(tempToMain,(char*)&dataToSend,sizeof(mainStruct),0);    
    pthread_exit(NULL);
}

static void* lightSensorRoutine(void *dataObj)
{
    printf("Entered lightSensorRoutine\n");

    mainStruct dataToSend;
    dataToSend.data=22;
    mqd_t lightToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
    if(lightToMain < 0)
    {
        perror("Main queue creation failed");
    }
    mq_send(lightToMain,(char*)&dataToSend,sizeof(mainStruct),0);

    pthread_exit(NULL);
}

static void* socketRoutine(void *dataObj)
{
    printf("Entered socketRoutine\n");
    pthread_exit(NULL);
}

static void* loggerRoutine(void *dataObj)
{
    printf("Entered loggerRoutine\n");
    pthread_exit(NULL);
}


int main()
{
    mainStruct dataToReceive;
    mainInfoToOthers dataObj;

  
    mqd_t mainQueue = mqueue_init(MAINQUEUENAME, MAIN_QUEUE_SIZE, sizeof(mainStruct));
    if(mainQueue < 0)
    {
        perror("Main queue creation failed");
    }

    mqd_t tempQueue = mqueue_init(TEMPQUEUENAME, TEMP_QUEUE_SIZE, sizeof(mainStruct));
    if(tempQueue < 0)
    {
        perror("Main queue creation failed");
    }

    mqd_t lightQueue = mqueue_init(LIGHTQUEUENAME, LIGHT_QUEUE_SIZE, sizeof(mainStruct));
    if(lightQueue < 0)
    {
        perror("Main queue creation failed");
    }


    
    //* FIll the data in the structure to send.

    printf("Main thread created with PID: %d and TID: %d\n",getpid(),(pid_t)syscall(SYS_gettid));

    /*Creating Temperature Sensor Thread */
    if(pthread_create(&tempSensorTask,NULL,&tempSensorRoutine,(void *)&dataObj)!=0)
    {
        perror("tempSensorTask create failed");
    }
    
    /*Creating Light Sensor Thread */
    if(pthread_create(&lightSensorTask,NULL,&lightSensorRoutine,(void *)&dataObj)!=0)
    {
        perror("lightSensorTask create failed");
    }

    /*Creating Logger Thread */
    if(pthread_create(&loggerTask,NULL,&loggerRoutine,(void *)&dataObj)!=0)
    {
        perror("loggerTask create failed");
    }

    /*Creating Socket Thread */
    if(pthread_create(&socketTask,NULL,&socketRoutine,(void *)&dataObj)!=0)
    {
        perror("socketTask create failed");
    }

    while(1)
    {
        int ret=mq_receive(mainQueue, (char*)&dataToReceive, sizeof(mainStruct),0);
        if(ret==-1)
        {
            perror("Error receving data from main queue");
        }
        printf("Data received from main queue %d\n",dataToReceive.data);
    }
    
    pthread_join(tempSensorTask,NULL);
    pthread_join(lightSensorTask,NULL);
    pthread_join(loggerTask,NULL);
    pthread_join(socketTask,NULL);

    return 0;
}

