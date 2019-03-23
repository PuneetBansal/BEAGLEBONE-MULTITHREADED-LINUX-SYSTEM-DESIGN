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
static void signal_handler(int , siginfo_t *, void*);

static void* tempSensorRoutine(void *dataObj)
{
    printf("Entered tempSensorTask\n");
    mainStruct dataToSend;
    dataToSend.data=55;
    mqd_t tempToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
    if(tempToMain < 0)
    {
		//printf("%s",__func__);
        perror("Temp queue creation failed");
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
		//printf("%s",__func__);
        perror("Light queue creation failed");
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
        perror("Maintemp queue creation failed");
    }

    mqd_t lightQueue = mqueue_init(LIGHTQUEUENAME, LIGHT_QUEUE_SIZE, sizeof(mainStruct));
    if(lightQueue < 0)
    {
        perror("Mainlight queue creation failed");
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

	//Creating 4 timer for monoring 4 created tasks.
	struct sigevent sigevTemp, sigevLight, sigevLog, sigevSocket;
	struct itimerspec timerConfigTemp, timerConfigLight, timerConfigLog, timerConfigSocket;
	timer_t timerTemp, timerLight, timerSocket, timerLog;
	struct sigaction sigact;

	sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = signal_handler;
	if((sigaction(SIGRTMIN, &sigact, NULL))<0)
	{
		perror("Failed setting signal handler");
	}
	
	if((sigaction(SIGRTMIN + 1, &sigact, NULL))<0)
	{
		perror("Failed setting signal handler");
	}
	
	if((sigaction(SIGRTMIN + 2, &sigact, NULL))<0)
	{
		perror("Failed setting signal handler");
	}
	
	if((sigaction(SIGRTMIN + 3, &sigact, NULL))<0)
	{
		perror("Failed setting signal handler");
	}

	sigevTemp.sigev_notify            = SIGEV_SIGNAL;
	sigevTemp.sigev_signo             = SIGRTMIN;
	sigevTemp.sigev_value.sival_ptr   = &timerTemp;
	
	sigevLight.sigev_notify           = SIGEV_SIGNAL;
	sigevLight.sigev_signo            = SIGRTMIN + 1;
	sigevLight.sigev_value.sival_ptr  = &timerLight;
   
	sigevSocket.sigev_notify          = SIGEV_SIGNAL;
	sigevSocket.sigev_signo           = SIGRTMIN + 2;
	sigevSocket.sigev_value.sival_ptr = &timerSocket;

	sigevLog.sigev_notify             = SIGEV_SIGNAL;
	sigevLog.sigev_signo              = SIGRTMIN + 3;
	sigevLog.sigev_value.sival_ptr    = &timerLog;

	if((timer_create(CLOCK_REALTIME, &sigevTemp, &timerTemp)) < 0)
	{
		perror("Temp Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevLight, &timerLight)) < 0)
	{
		perror("Light Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevLog, &timerLog)) < 0)
	{
		perror("Log Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevSocket, &timerSocket)) < 0)
	{
		perror("Socket Timer setup failed");
		exit(1);
	}

	timerConfigTemp.it_interval.tv_nsec = 0;
	timerConfigTemp.it_interval.tv_sec  = 0;
	timerConfigTemp.it_value.tv_nsec    = 10000000;
	timerConfigTemp.it_value.tv_sec     = 0;

	timerConfigLight.it_interval.tv_nsec = 0;
	timerConfigLight.it_interval.tv_sec  = 0;
	timerConfigLight.it_value.tv_nsec    = 0;//10000000;
	timerConfigLight.it_value.tv_sec     = 3;

	timerConfigLog.it_interval.tv_nsec = 0;
	timerConfigLog.it_interval.tv_sec  = 0;
	timerConfigLog.it_value.tv_nsec    = 10000000;
	timerConfigLog.it_value.tv_sec     = 0;

	timerConfigSocket.it_interval.tv_nsec = 0;
	timerConfigSocket.it_interval.tv_sec  = 0;
	timerConfigSocket.it_value.tv_nsec    = 10000000;
	timerConfigSocket.it_value.tv_sec     = 0;

	if((timer_settime(timerTemp, 0, &timerConfigTemp, NULL)) < 0)
	{
		perror("Temp Timer set failed");
		exit(1);
	}
	if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
	{
		perror("Light Timer set failed");
		exit(1);
	}
	if((timer_settime(timerLog, 0, &timerConfigLog, NULL)) < 0)
	{
		perror("Log Timer set failed");
		exit(1);
	}
	if((timer_settime(timerSocket, 0, &timerConfigSocket, NULL)) < 0)
	{
		perror("Socket Timer set failed");
		exit(1);
	}

	
    while(1)
    {
        int ret=mq_receive(mainQueue, (char*)&dataToReceive, sizeof(mainStruct),0);
        if(ret==-1)
        {
            perror("Error receving data from main queue");
        }
        printf("Data received from main queue %d\n",dataToReceive.data);
		if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
		{
			perror("Light Timer set failed");
			exit(1);
		}
    }
    
    pthread_join(tempSensorTask,NULL);
    pthread_join(lightSensorTask,NULL);
    pthread_join(loggerTask,NULL);
    pthread_join(socketTask,NULL);

    return 0;
}


static void signal_handler(int sig, siginfo_t *si, void *uc)
{
	switch(sig)
	{
	case 34:
		printf("SIGRTMIN signal is received\n");
		break;
	case 35:
		printf("SIGRTMIN+1 signal is received\n");
		break;
	case 36:
		printf("SIGRTMIN+2 signal is received\n");
		break;
	case 37:
		printf("SIGRTMIN+3 signal is received\n");
		break;
	}
}
