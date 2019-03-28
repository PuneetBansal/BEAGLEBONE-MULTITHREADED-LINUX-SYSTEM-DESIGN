#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>
#include<sys/socket.h>
#include<netinet/in.h>

/*User defined headers*/
#include "maintask.h"

pthread_t tempSensorTask,lightSensorTask,loggerTask,socketTask;
static void signal_handler(int , siginfo_t *, void*);
bool measureTemperature = true;
bool measureLight=false;
bool loggerHeartBeat=false;
bool socketHeartBeat=false;

static void* tempSensorRoutine(void *dataObj)
{
	mainStruct dataToSendToMain;
	logStruct dataToSendToLogger;
	socketStruct dataToSendToSocket;
	tempStruct dataReceived;

	int init_status = init_success;
	struct sigevent tempEvent;
	struct sigaction tempAction;
	struct itimerspec timerSpec;
	timer_t tempTimer;
	bool sendTempToSocket = false;

	printf("Entered tempSensorTask\n");
	//* Initialize temperature sensor here with config register, TLOW and THIGH register values.
	//* if init fails init_status = fail else init_status = success;
    
	strcpy(dataToSendToMain.source,"temperature");
	dataToSendToMain.status       = init_status; //* We are to send the init status only after thi task is properly initialised.
	dataToSendToMain.messageType  = update;
	
	//printf("Source test = %s - %ld\n",dataToSend.source,sizeof(dataToSend));
    mqd_t tempToMain = mqueue_init(MAINQUEUENAME, MAIN_QUEUE_SIZE, sizeof(dataToSendToMain));
	mqd_t tempToLogger = mqueue_init(LOGQUEUENAME, LOG_QUEUE_SIZE, sizeof(dataToSendToLogger));
	mqd_t tempQueue = mqueue_init(TEMPQUEUENAME, TEMP_QUEUE_SIZE, sizeof(dataReceived));
	mqd_t socketQueue = mqueue_init(SOCKETQUEUENAME, SOCKET_QUEUE_SIZE, sizeof(dataToSendToSocket));    

	if(tempToMain < 0)
    {
		//printf("%s",__func__);
        perror("TemptoMain queue creation failed");
    }
	if(tempToLogger <0)
	{
		perror("TempToLog queue creation failed");
	}
	if(tempQueue < 0)
    {
        perror("Temp queue creation failed");
    }
	if(socketQueue < 0)
    {
        perror("socketQueue creation failed");
    }
	
	int x = mq_send(tempToMain,(char*)&dataToSendToMain,sizeof(dataToSendToMain),0);
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
	timerSpec.it_interval.tv_nsec = 0;//100000000; //To get the value from sensor every 100 ms
	timerSpec.it_interval.tv_sec  = 2;
	timerSpec.it_value.tv_nsec    = 100000000;
	timerSpec.it_value.tv_sec     = 0;

	if((timer_settime(tempTimer, 0, &timerSpec, NULL)) < 0)
	{
		perror("Starting timer in temperature task failed");
		exit(1);
	}
	
	strcpy(dataToSendToMain.source,"temperature");
	int y = 0;
	while(1)
	{
		if(measureTemperature)
		{
			int ret = mq_receive(tempQueue,(char*)&dataReceived,sizeof(dataReceived),0);
			if(ret < 0)
			{
				//perror("No requests received");
			}
			else
			{
				printf("Task succeded\n");
				sendTempToSocket = true;
			}
			//printf("Sending Temperature\n");
			//Take temperaure measurement
			// Send it to logger.
			measureTemperature = false;
			//Sending heartbeat message
			dataToSendToMain.messageType = update;
			dataToSendToMain.status = success;
			
			dataToSendToLogger.status = success;
			dataToSendToLogger.source= "temperature";
			dataToSendToLogger.value = y;

			mq_send(tempToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);
			mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
			if(sendTempToSocket)
			{
				printf("Sending data to socket\n");
				sendTempToSocket = false;
				dataToSendToSocket.source = "temperature";
				// read temperature value.
				dataToSendToSocket.value  = y;
				dataToSendToSocket.unit = dataReceived.unit;
				int ret = mq_send(socketQueue, (char*)&dataToSendToSocket, sizeof(dataToSendToSocket),0);
				if (ret < 0)
				{
					perror("Temperature sending to socket failed");
				}
			}
			y++;
		}
		//if(y > 3)
		//	break;
	}
	timer_delete(tempTimer);
    pthread_exit(NULL);
}

static void* lightSensorRoutine(void *dataObj)
{
    printf("Entered lightSensorRoutine\n");

    mainStruct dataToSendToMain;
	logStruct dataTOSendTOLogger;
	 
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


	dataToSendToMain.status = init_success;
    mqd_t lightToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(dataToSendToMain));
	mqd_t lightToLogger = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));
    if(lightToMain < 0)
    {
		//printf("%s",__func__);
        perror("Light queue creation failed");
    }
    if(lightToLogger<0)
	{
		perror("LightToLogger queue creation failed");
	}

	mq_send(lightToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);
    
 int y=0;

 while(1)
 {
	if(measureLight)
	{
		//printf("Sending light value to logger and updating heart beat to main\n");
		measureLight=false;
		//take the light sensor value here and send it to logger task
		dataToSendToMain.messageType = update;
		dataToSendToMain.status = success;	
		
		dataTOSendTOLogger.status = success;
		dataTOSendTOLogger.source= "light";
		dataTOSendTOLogger.value = y;
		
		printf("Sending light heartbeat");
		mq_send(lightToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);	
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		y++;
	}
 //if(y>3)
 //	break;
 }

    timer_delete(lightTimer);
	pthread_exit(NULL);
}

static void* socketRoutine(void *dataObj)
{
    printf("Entered socketRoutine\n");
	int serverSocket,clientConnected,len;
	struct sockaddr_in clientAddr,serverAddr;
	struct hostent *ptr;
	int n=0; 
	
	mainStruct dataToSendToMain;
	logStruct dataTOSendTOLogger;
	socketStruct dataReceivedFromSensors;
	 
	/*Initialising Timer*/
    struct sigevent socketEvent;	
	struct sigaction socketAction;
	struct itimerspec socketTimerSpec;
	timer_t socketTimer;

	socketAction.sa_flags = SA_SIGINFO;
	socketAction.sa_sigaction = signal_handler;		
	
	if(sigaction(SIGRTMIN + 7 , &socketAction, NULL)<0)
	{
		perror("Light sensor, timer handler initialisation failed");
	}	
	
	socketEvent.sigev_notify = SIGEV_SIGNAL;
    socketEvent.sigev_signo = SIGRTMIN + 7;
    socketEvent.sigev_value.sival_ptr = &socketTimer;

	if((timer_create(CLOCK_REALTIME, &socketEvent, &socketTimer)) < 0)
	{
		perror("Timer creation failed for temperature task");
		exit(1);
	}

	socketTimerSpec.it_interval.tv_nsec = 0;//100000000; //To get the heart beat value after every 1 s
	socketTimerSpec.it_interval.tv_sec  = 1;
	socketTimerSpec.it_value.tv_nsec    = 0;//100000000;
	socketTimerSpec.it_value.tv_sec     = 1;

	if(timer_settime(socketTimer,0,&socketTimerSpec,NULL)<0)
	{
		perror("Light sensor timer settime failed");
		exit(1);
	}


    strcpy(dataToSendToMain.source,"socket");
	dataToSendToMain.messageType = update;
	dataToSendToMain.status = init_success;

    mqd_t socketToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
	mqd_t socketToLogger = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));
    mqd_t sensorToSocket = mqueue_init(SOCKETQUEUENAME,SOCKET_QUEUE_SIZE,sizeof(socketStruct));
	
	if(socketToMain < 0)
    {
		//printf("%s",__func__);
        perror("Light queue creation failed");
    }
    if(socketToLogger<0)
	{
		perror("LightToLogger queue creation failed");
	}

	 if(sensorToSocket<0)
	{
		perror("Sensor to socket queue creation failed");
		printf("Socket queue creation failed");
	}
	

	mq_send(socketToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);

	
	/*Setting up server socket*/
	serverSocket=socket(AF_INET, SOCK_STREAM, 0);

	memset((char*)&serverAddr,0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(10000);

	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(serverSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr)) == -1)
	printf("Bind Failure\n");
	else
	printf("Bind Success:<%u>\n", serverSocket);

	listen(serverSocket,5);
	len=sizeof(struct sockaddr_in);

	clientConnected=accept(serverSocket,(struct sockaddr*)&clientAddr,&len);
	if (clientConnected !=-1)
	printf("Connection accepted:<%u>\n", clientConnected);
	
	int input;

	while(1)
	{
		int rb=read(clientConnected,&input, sizeof(input));
		if(rb==sizeof(input))
		{
			printf("Message received from client is %d\n",input);
			strcpy(dataToSendToMain.source,"socket");
			dataToSendToMain.messageType= request;
			switch(input)
			{
				case 1:
				strcpy(dataToSendToMain.unit,"Celsius");
				break;

				case 2:
				strcpy(dataToSendToMain.unit,"Kelvin");
				break;

				case 3:
				strcpy(dataToSendToMain.unit,"Fahrenheit");
				break;

				case 4:
				strcpy(dataToSendToMain.unit,"Light");
				break;
				
			}
		mq_send(socketToMain,(char *) &dataToSendToMain,sizeof(mainStruct),0);
		}	

		if(socketHeartBeat)
		{
		socketHeartBeat=false;
		strcpy(dataToSendToMain.source,"socket");
		dataToSendToMain.messageType = update;
		dataToSendToMain.status = success;		
		} 
		const int errval=-500;

		if(mq_receive(sensorToSocket,(char*)&dataReceivedFromSensors,sizeof(socketStruct),0)>-1)
		{
			printf("Data received by server queue from %s\n",dataReceivedFromSensors.source);
			if(dataReceivedFromSensors.source=="temperature")
			{
				printf("Temperaturee readings received from Temp task to socket server\n");
				if(send(clientConnected, (void*)&dataReceivedFromSensors.value,sizeof(dataReceivedFromSensors.value),0)!=sizeof(dataReceivedFromSensors.value))
				{
				printf("Sending temp value to client failed\n");
				perror("Sending temp value to client failed\n");
				}
			}
			else if(dataReceivedFromSensors.source=="light")
			{
				if(send(clientConnected, (void*)&dataReceivedFromSensors.value,sizeof(dataReceivedFromSensors.value),0)!=sizeof(dataReceivedFromSensors.value))
				{
				printf("Sending temp value to client failed\n");
				perror("Sending temp value to client failed\n");
				}
			}
			else
			{
				strcpy(dataTOSendTOLogger.message,dataReceivedFromSensors.message);
				strcpy(dataTOSendTOLogger.source,"socket");
				dataTOSendTOLogger.status=fail;
				mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);

				//log the error message to the log file and send it to the client.]

				if(send(clientConnected, (void*)&errval,sizeof(errval),0)!=sizeof(errval))
				{
				printf("Sending error value to client failed\n");
				perror("Sending error value to client failed\n");
				}
			}
			
		}
	}
	close(serverSocket);
	printf("\nServer Socket Closed !!\n");
	pthread_exit(NULL);
	}

static void* loggerRoutine(void *dataObj)
{
    printf("Entered loggerRoutine\n");
    printf("Entered lightSensorRoutine\n");

    mainStruct dataToSend;
	logStruct dataToReceive;
	 
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
	mqd_t loggerQueue = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));
	
	if(loggerQueue < 0)
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
		//else if (y>3)
		//{
		//	break;			
		//}
	 
	 	else
		 {
			 int ret= mq_receive(loggerQueue, (char*)&dataToReceive, sizeof(logStruct),0);
			 //printf("return value of mq_receive in logger is %d\n",ret);
			if(ret!=-1)
			{
				//printf("Enteref if of else in logger task (before logtofile)");
				//printf("source in logger queue is %s\n",dataToReceive.source);
				//printf("Received value is %f \n",dataToReceive.value);
			 	logToFile(dataToReceive);	
			}
		 }
		 
	}
    timer_delete(loggerTimer);
	pthread_exit(NULL);
}

uint8_t isAlive = 0xFF;
int main()
{
    mainStruct dataToReceive;
    mainInfoToOthers dataObj;
	tempStruct requestForTemp;
	socketStruct responseToSocket;
	lightStruct requestForLight;
	
  
    mqd_t mainQueue = mqueue_init(MAINQUEUENAME, MAIN_QUEUE_SIZE, sizeof(mainStruct));
    if(mainQueue < 0)
    {
        perror("Main queue creation failed");
    }

    mqd_t tempQueue = mqueue_init(TEMPQUEUENAME, TEMP_QUEUE_SIZE, sizeof(tempStruct));
    if(tempQueue < 0)
    {
        perror("Temp queue creation failed");
    }

    mqd_t lightQueue = mqueue_init(LIGHTQUEUENAME, LIGHT_QUEUE_SIZE, sizeof(lightStruct));
    if(lightQueue < 0)
    {
        perror("Light queue init failed");
    }
	
	mqd_t socketQueue = mqueue_init(SOCKETQUEUENAME, SOCKET_QUEUE_SIZE, sizeof(socketStruct));
	if(socketQueue < 0)
	{
		perror("Socket queue creation failed");
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

	sigevLog.sigev_notify             = SIGEV_SIGNAL;
	sigevLog.sigev_signo              = SIGRTMIN + 2;
	sigevLog.sigev_value.sival_ptr    = &timerLog;   

	sigevSocket.sigev_notify          = SIGEV_SIGNAL;
	sigevSocket.sigev_signo           = SIGRTMIN + 3;
	sigevSocket.sigev_value.sival_ptr = &timerSocket;

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
	timerConfigTemp.it_value.tv_sec     = 5;

	timerConfigLight.it_interval.tv_nsec = 0;
	timerConfigLight.it_interval.tv_sec  = 0;
	timerConfigLight.it_value.tv_nsec    = 0;
	timerConfigLight.it_value.tv_sec     = 1;

	timerConfigLog.it_interval.tv_nsec = 0;
	timerConfigLog.it_interval.tv_sec  = 0;
	timerConfigLog.it_value.tv_nsec    = 0;
	timerConfigLog.it_value.tv_sec     = 6; //logger task updates heart beat in every 2 second. if it doesn't update heart beat within 6 seconds, it will result in timeout

	timerConfigSocket.it_interval.tv_nsec = 0;
	timerConfigSocket.it_interval.tv_sec  = 0;
	timerConfigSocket.it_value.tv_nsec    = 0;
	timerConfigSocket.it_value.tv_sec     = 3;  //socket task updates heart beat in every 1 second. if it doesn't update heart beat within 3 seconds, it will result in timeout

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
			//printf("Data source from main queue %s\n",dataToReceive.source);
			//printf("Data success = %d\n",dataToReceive.status);
			//printf("Data mesage_type = %d\n",dataToReceive.messageType);
			if((strcmp(dataToReceive.source,"temperature"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					//printf("Init_success message received\n");
					isAlive = isAlive | TEMPERATURE_TASK;
					break;
				case success:
					//printf("Success message is received\n");
					if((timer_settime(timerTemp, 0, &timerConfigTemp, NULL)) < 0)
					{
						perror("Temp Timer set failed");
					}
					break;
				case fail:
					//printf("Fail message is received\n");
					isAlive = isAlive & ~TEMPERATURE_TASK;
					break;
				case init_failure:
					//printf("Init_failure message received\n");
					isAlive = isAlive & ~TEMPERATURE_TASK;
					break;
				}
			}
			else if((strcmp(dataToReceive.source,"light"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					//printf("Init_success message received\n");
					isAlive = isAlive | LIGHT_TASK;
					break;
				case success:
					//printf("Success message is received\n");
					if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
					{
						perror("Light Timer set failed");
					}
					break;
				case fail:
					//printf("Fail message is received\n");
					isAlive = isAlive & ~LIGHT_TASK;
					break;
				case init_failure:
					//printf("Init_failure message received\n");
					isAlive = isAlive & ~LIGHT_TASK;
					break;
				}
			}
			else if((strcmp(dataToReceive.source,"logger"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					//printf("Init_success message received\n");
					isAlive = isAlive | LOGGER_TASK;
					break;
				case success:
					//printf("Success message is received\n");
					if((timer_settime(timerLog, 0, &timerConfigLog, NULL)) < 0)
					{
						perror("Logger Timer set failed");
					}
					break;
				case fail:
					//printf("Fail message is received\n");
					isAlive = isAlive & ~LOGGER_TASK;
					break;
				case init_failure:
					//printf("Init_failure message received\n");
					isAlive = isAlive & ~LOGGER_TASK;
					break;
				}
			}
			else if((strcmp(dataToReceive.source,"socket"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					//printf("Init_success message received\n");
					isAlive = isAlive | SOCKET_TASK;
					break;
				case success:
					//printf("Success message is received\n");
					if((timer_settime(timerSocket, 0, &timerConfigSocket, NULL)) < 0)
					{
						perror("Socket Timer set failed");
					}
					break;
				case fail:
					//printf("Fail message is received\n");
					isAlive = isAlive & ~SOCKET_TASK;
					break;
				case init_failure:
					//printf("Init_failure message received\n");
					isAlive = isAlive & ~SOCKET_TASK;
					break;
				}
			}
			else if((strcmp(dataToReceive.source,"socket"))==0 && dataToReceive.messageType == request)
			{
				printf("__func() Message received from server to main\n");
				printf("__func() source : %s\n",dataToReceive.source);
				printf("__func() unit : %s\n",dataToReceive.unit);

				if(strcmp(dataToReceive.unit,"Celsius")==0 || strcmp(dataToReceive.unit,"Fahrenheit")==0 || strcmp(dataToReceive.unit,"Kelvin")==0)
				{
				// check isAlive - if temp task is alive populate and send message to Temperature task
					if(isAlive & TEMPERATURE_TASK)
					{
						printf("\n__func() Temp task is alive\n");
						printf("__func() sending request to temp sensor task to send data to socket task\n");
						strcpy(requestForTemp.source, "main");
						strcpy(requestForTemp.unit, dataToReceive.unit);
						int noOfBytesSent = mq_send(tempQueue, (char*)&requestForTemp, sizeof(requestForTemp), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending request failed");
							printf("Sending temperature reuest from main task failed\n");
						}
					//printf("Sending temperature reuest from main task successfull\n");
					}
				// else send temp task not active message to socket task
					else
					{
						printf("__func() temp sensor task is not active\n");
						strcpy(responseToSocket.source, "main");
						int noOfBytesSent = mq_send(socketQueue, (char*)&responseToSocket, sizeof(responseToSocket), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending response failed");
						}
					}
				}
				else
				{
				// check isAlive - if light task is alive populate and send message to Light task
				// else send light task not active message to socket task
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
		// Siganl indicating temperature sensor timeout i.e. Temperature task failed
		printf("SIGRTMIN(TemperatureTaskTimeout) signal is received\n");
		isAlive &= ~TEMPERATURE_TASK;
		break;
	case 35:
		// Signal indicating light sensor timeout i.e. Light task failed
		printf("SIGRTMIN+1(LightTaskTimeout) signal is received\n");
		isAlive &= ~LIGHT_TASK;
		break;
	case 36:
		// Signal indicating logger timeout i.e. Logger task failed
		printf("SIGRTMIN+2(LoggerTaskTimeout) signal is received\n");
		isAlive &= ~LOGGER_TASK;
		break;
	case 37:
		// Signal indicating socket sensor timeout i.e. Socket task failed
		printf("SIGRTMIN+3(SocketTaskTimeout) signal is received\n");
		isAlive &= ~SOCKET_TASK;
		break;
	case 38:
		// Signal instructing to take temperature sensor reading and send heartbeat
		//printf("SIGRTMIN+4(TempTimeHandler) signal is received\n");
		measureTemperature = true;
		break;
	case 39:
		// Signal instructing to take light sensor reading and send heartbeat
		printf("SIGRTMIN+5(lighTimerHandler) signal is received\n");
		measureLight = true;
		break;
	case 40:
		// Signal instructing logger task to send heartbeat to main indicating it is alive
		//printf("SIGRTMIN+6(LoggerTimerHandler) signal is received\n");
		loggerHeartBeat = true;
		break;
	case 41:
		//printf("SIGRTMIN+7(SocketTimerHandler) signal is received\n");
		socketHeartBeat = true;
		break;
	}

}
