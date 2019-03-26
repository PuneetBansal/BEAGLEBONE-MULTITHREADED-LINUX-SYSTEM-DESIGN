#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

/*User defined headers*/
#include "maintask.h"

pthread_t tempSensorTask,lightSensorTask,loggerTask,socketTask;
static void signal_handler(int , siginfo_t *, void*);
bool measureTemperature = true;
bool measureLight=false;
bool loggerHeartBeat=false;

static void* tempSensorRoutine(void *dataObj)
{
	mainStruct dataToSend;
	int init_status = init_success;
	struct sigevent tempEvent;
	struct sigaction tempAction;
	struct itimerspec timerSpec;
	timer_t tempTimer;
	printf("Entered tempSensorTask\n");
	//* Initialize temperature sensor here with config register, TLOW and THIGH register values.
	//* if init fails init_status = fail else init_status = success;
    strcpy(dataToSend.source,"temperature");
	dataToSend.status       = init_status; //* We are to send the init status only after thi task is properly initialised.
	dataToSend.messageType  = update;
	
	//printf("Source test = %s - %ld\n",dataToSend.source,sizeof(dataToSend));
    mqd_t tempToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(dataToSend));
    if(tempToMain < 0)
    {
		//printf("%s",__func__);
        perror("Temp queue creation failed");
    }
    int x = mq_send(tempToMain,(char*)&dataToSend,sizeof(dataToSend),0);
	if(x < 0)
	{
		perror("Sending data failed");
	}
	
	//if init_status == fail then pthread exit after sending the message to main.   

	//Register signal handler for a signal
	tempAction.sa_flags     = SA_SIGINFO;
    tempAction.sa_sigaction = signal_handler;
	if((sigaction(SIGRTMIN + 4, &tempAction, NULL))<0)
	{
		perror("Failed setting signal handler for temp measurement");
	}
	
	//Assigning signal to timer
	tempEvent.sigev_notify             = SIGEV_SIGNAL;
	tempEvent.sigev_signo              = SIGRTMIN + 4;
	tempEvent.sigev_value.sival_ptr    = &tempTimer;
	if((timer_create(CLOCK_REALTIME, &tempEvent, &tempTimer)) < 0)
	{
		perror("Timer creation failed for temperature task");
		exit(1);
	}

	//Setting the time and starting the timer
	timerSpec.it_interval.tv_nsec = 100000000; //To get the value from sensor every 100 ms
	timerSpec.it_interval.tv_sec  = 0;
	timerSpec.it_value.tv_nsec    = 100000000;
	timerSpec.it_value.tv_sec     = 0;

	if((timer_settime(tempTimer, 0, &timerSpec, NULL)) < 0)
	{
		perror("Starting timer in temperature task failed");
		exit(1);
	}
	
	strcpy(dataToSend.source,"temperature");
	int y = 0;
	while(1)
	{
		if(measureTemperature)
		{
			printf("Sending Temperature\n");
			//Take temperaure measurement
			// Send it to logger.
			measureTemperature = false;
			//Sending heartbeat message
			dataToSend.messageType = update;
			dataToSend.status = y;
			y++;
			mq_send(tempToMain,(char*)&dataToSend,sizeof(mainStruct),0);
			if(y > 3)
				break;
		}
	}
	timer_delete(tempTimer);
    pthread_exit(NULL);
}

static void* lightSensorRoutine(void *dataObj)
{
    printf("Entered lightSensorRoutine\n");

    mainStruct dataToSend;
	 
	/*Initialising Timer*/
    struct sigevent lightEvent;	
	struct sigaction lightAction;
	struct itimerspec lightTimerSpec;
	timer_t lightTimer;

	lightAction.sa_flags = SA_SIGINFO;
	lightAction.sa_sigaction = signal_handler;		
	
	if(sigaction(SIGRTMIN + 5 , &lightAction, NULL)<0)
	{
		perror("Light sensor, timer handler initialisation failed");
	}	
	
	lightEvent.sigev_notify = SIGEV_SIGNAL;
    lightEvent.sigev_signo = SIGRTMIN + 5;
    lightEvent.sigev_value.sival_ptr = &lightTimer;

	if((timer_create(CLOCK_REALTIME, &lightEvent, &lightTimer)) < 0)
	{
		perror("Timer creation failed for temperature task");
		exit(1);
	}

	lightTimerSpec.it_interval.tv_nsec = 100000000; //To get the value after every 100 ms
	lightTimerSpec.it_interval.tv_sec  = 0;
	lightTimerSpec.it_value.tv_nsec    = 100000000;
	lightTimerSpec.it_value.tv_sec     = 0;

	if(timer_settime(lightTimer,0,&lightTimerSpec,NULL)<0)
	{
		perror("Light sensor timer settime failed");
		exit(1);
	}


    strcpy(dataToSend.source,"light");
	dataToSend.messageType = update;
	dataToSend.status = init_success;
    mqd_t lightToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
    if(lightToMain < 0)
    {
		//printf("%s",__func__);
        perror("Light queue creation failed");
    }
    mq_send(lightToMain,(char*)&dataToSend,sizeof(mainStruct),0);
    
 int y=0;

 while(1)
 {
	if(measureLight)
	{
		printf("Sednign light value to logger and updating heart beat to main\n");
		measureLight=false;
		//take the light sensor value here and send it to logger task
		dataToSend.messageType = update;
		dataToSend.status = success;	
		mq_send(lightToMain,(char*)&dataToSend,sizeof(mainStruct),0);	
		y++;
	}
 if(y>3)
 break;
 }

    timer_delete(lightTimer);
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
    printf("Entered lightSensorRoutine\n");

    mainStruct dataToSend;
	 
	/*Initialising Timer*/
    struct sigevent loggerEvent;	
	struct sigaction loggerAction;
	struct itimerspec loggerTimerSpec;
	timer_t loggerTimer;

	loggerAction.sa_flags = SA_SIGINFO;
	loggerAction.sa_sigaction = signal_handler;		
	
	if(sigaction(SIGRTMIN + 6 , &loggerAction, NULL)<0)
	{
		perror("Logger, timer handler initialisation failed");
	}	
	
	loggerEvent.sigev_notify = SIGEV_SIGNAL;
    loggerEvent.sigev_signo = SIGRTMIN + 6;
    loggerEvent.sigev_value.sival_ptr = &loggerTimer;

	if((timer_create(CLOCK_REALTIME, &loggerEvent, &loggerTimer)) < 0)
	{
		perror("Timer creation failed for logger task");
		exit(1);
	}

	loggerTimerSpec.it_interval.tv_nsec = 0;//100000000; //To get the value after every 100 ms
	loggerTimerSpec.it_interval.tv_sec  = 2;
	loggerTimerSpec.it_value.tv_nsec    = 0;//100000000;
	loggerTimerSpec.it_value.tv_sec     = 2;

	if(timer_settime(loggerTimer,0,&loggerTimerSpec,NULL)<0)
	{
		perror("logger timer settime failed");
		exit(1);
	}


    strcpy(dataToSend.source,"logger");
	dataToSend.messageType = update;
	dataToSend.status = init_success;
    mqd_t loggerToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
    if(loggerToMain < 0)
    {
		//printf("%s",__func__);
        perror("logger queue creation failed");
    }
    mq_send(loggerToMain,(char*)&dataToSend,sizeof(mainStruct),0);
    
 int y=0;

 while(1)
 {
	if(loggerHeartBeat)
	{
		printf("Updating logger heart beat to main\n");
		loggerHeartBeat=false;
		
		dataToSend.messageType = update;
		dataToSend.status = success;	
		mq_send(loggerToMain,(char*)&dataToSend,sizeof(mainStruct),0);	
		y++;
	}
 if(y>3)
 break;
 }

    timer_delete(loggerTimer);
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
	printf("Check this - %p <-------------------------\nf",&timerTemp);
	
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
	timerConfigTemp.it_value.tv_nsec    = 0;
	timerConfigTemp.it_value.tv_sec     = 1;

	timerConfigLight.it_interval.tv_nsec = 0;
	timerConfigLight.it_interval.tv_sec  = 0;
	timerConfigLight.it_value.tv_nsec    = 0;
	timerConfigLight.it_value.tv_sec     = 1;

	timerConfigLog.it_interval.tv_nsec = 0;
	timerConfigLog.it_interval.tv_sec  = 0;
	timerConfigLog.it_value.tv_nsec    = 0;
	timerConfigLog.it_value.tv_sec     = 6;

	timerConfigSocket.it_interval.tv_nsec = 0;
	timerConfigSocket.it_interval.tv_sec  = 0;
	timerConfigSocket.it_value.tv_nsec    = 0;
	timerConfigSocket.it_value.tv_sec     = 1;

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
		//printf("Receivin\n");
        int ret=mq_receive(mainQueue, (char*)&dataToReceive, sizeof(mainStruct),0);
        if(ret>-1)
        {
			printf("Data source from main queue %s\n",dataToReceive.source);
			printf("Data success = %d\n",dataToReceive.status);
			printf("Data mesage_type = %d\n",dataToReceive.messageType);
			if((strcmp(dataToReceive.source,"temperature"))==0 && dataToReceive.messageType == update && dataToReceive.status == success)
			{
				if((timer_settime(timerTemp, 0, &timerConfigTemp, NULL)) < 0)
				{
					perror("Temp Timer set failed");
				}
			}
			else if((strcmp(dataToReceive.source,"light"))==0 && dataToReceive.messageType == update && dataToReceive.status == success)
			{
				if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
				{
					perror("Light Timer set failed");
				}
			}

			else if((strcmp(dataToReceive.source,"logger"))==0 && dataToReceive.messageType == update && dataToReceive.status == success)
			{
				if((timer_settime(timerLog, 0, &timerConfigLog, NULL)) < 0)
				{
					perror("Logger Timer set failed");
				}
			}

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
	case 38:
		printf("SIGRTMIN+4 signal is received\n");
		measureTemperature = true;
		break;
	case 39:
		printf("SIGRTMIN+5(lighTimerHandler) signal is received\n");
		measureLight = true;
		break;
	case 40:
		printf("SIGRTMIN+6(LoggerTimerHandler) signal is received\n");
		loggerHeartBeat = true;
		break;
	}
}
