/************************************************************************************************
* File name   : maintask.c                                                                      *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The main logic of the code                                                      *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
https://stackoverflow.com/questions/18021189/how-to-connect-two-computers-over-internet-using-socket-programming-in-c

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

/*User defined headers*/
#include "maintask.h"
#include "temp_i2c.h"
#include "lightsensor.h"
#include "gpio.h"
#include "bist.h"
#include "myi2c.h"

typedef struct 
{
  char *logFileName;
  int noOfParam;
}mainInfoToOthers;

typedef enum
{
    init,
    dark,
    light
}state;

int fd,fd1;

pthread_t tempSensorTask,lightSensorTask,loggerTask,socketTask;
static void signal_handler(int , siginfo_t *, void*);
bool measureTemperature = true;
bool measureLight=false;
bool loggerHeartBeat=false;
bool socketHeartBeat=false;
bool exitThread = false;


/**************************************************************************************
* Function name:- tempSensorRoutine                                                   *
* Description:- This function is used by a Temperature sensor thread for execution of *
*               temperature sensor task.                                              *
* @param:- void* (data from main)                                                     *
* @return:- static void* (pthread exit value)                                         *
**************************************************************************************/
static void* tempSensorRoutine(void *dataObj)
{
	mainStruct dataToSendToMain;
	logStruct dataToSendToLogger;
	socketStruct dataToSendToSocket;
	tempStruct dataReceived;
	int i2c_file_des;
	float temperature;
	uint16_t config;
	int gpio_val_fd;
	int gpio_state;
	struct pollfd fds;

	int init_status = init_success;
	struct sigevent tempEvent;
	struct sigaction tempAction;
	struct itimerspec timerSpec;
	timer_t tempTimer;
	bool sendTempToSocket = false;
    
	strcpy(dataToSendToMain.source,"temperature");
	dataToSendToMain.messageType  = update;

	/* Initialize gpio LED pin as output and pin 60 for interrupts */
	gpio_init(53, out);
	gpio_interrupt_state(60, both);
	gpio_val_fd = gpio_open_value(60);

	fds.fd = gpio_val_fd;
	fds.events = POLLPRI | POLLERR;
	
	/* Setting up all queues */
    mqd_t tempToMain = mqueue_init(MAINQUEUENAME, MAIN_QUEUE_SIZE, sizeof(dataToSendToMain));
	mqd_t tempToLogger = mqueue_init(LOGQUEUENAME, LOG_QUEUE_SIZE, sizeof(dataToSendToLogger));
	mqd_t tempQueue = mqueue_init(TEMPQUEUENAME, TEMP_QUEUE_SIZE, sizeof(dataReceived));
	mqd_t socketQueue = mqueue_init(SOCKETQUEUENAME, SOCKET_QUEUE_SIZE, sizeof(dataToSendToSocket));    

	if(tempToMain < 0)
    {
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
	
	dataToSendToLogger.logLevel = alert;		
	dataToSendToLogger.status = fail;
	dataToSendToLogger.source= "temperature";

	/* Register signal handler for a signal*/
	tempAction.sa_flags     = SA_SIGINFO;
    tempAction.sa_sigaction = signal_handler;
	if((sigaction(SIGRTMIN + 4, &tempAction, NULL))<0)
	{
		perror("Failed setting signal handler for temp measurement");
		dataToSendToLogger.message = "Signal handler set failed";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		init_status = init_failure;
	}
	
	/* Assigning signal to timer */
	tempEvent.sigev_notify             = SIGEV_SIGNAL;
	tempEvent.sigev_signo              = SIGRTMIN + 4;
	tempEvent.sigev_value.sival_ptr    = &tempTimer;
	if((timer_create(CLOCK_REALTIME, &tempEvent, &tempTimer)) < 0)
	{
		perror("Timer creation failed for temperature task");
		dataToSendToLogger.message = "Assigning signal failed";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		init_status = init_failure;
	}

	/* Setting the time and starting the timer */
	timerSpec.it_interval.tv_nsec = 200000000;
	timerSpec.it_interval.tv_sec  = 0;
	timerSpec.it_value.tv_nsec    = 200000000;
	timerSpec.it_value.tv_sec     = 0;

	if((timer_settime(tempTimer, 0, &timerSpec, NULL)) < 0)
	{
		perror("Starting timer in temperature task failed");
		dataToSendToLogger.message = "Failed to start timer";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		init_status = init_failure;
	}
	
	/* Open i2c file to start communication with temperature sensor. */
	
	i2c_file_des = fd1;
	
	/* Initialize temperature sensor lower limit alert register */
	int success = temp_i2c_write_to_reg(i2c_file_des, TLOW_REG_ADDR, 26);
	if(success < 0)
	{
		printf("Write failure\nExiting Temperature measurement task\n");
		dataToSendToLogger.message = "Write failure";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		init_status = init_failure;
		gpio_write_value(53,high);		
	}

	/* Initialize temperature sensor higer limit alert register. */
	success = temp_i2c_write_to_reg(i2c_file_des, THIGH_REG_ADDR, 30);
	if(success < 0)
	{
		printf("Write failure\nExiting Temperature measurement task\n");
		dataToSendToLogger.message = "Write failure";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		init_status = init_failure;
		gpio_write_value(53,high);	
	}
	
	/* Send inititalization success/fail message to main. */
	dataToSendToMain.status = init_status;
	int x = mq_send(tempToMain,(char*)&dataToSendToMain,sizeof(dataToSendToMain),0);
	if(x < 0)
	{
		perror("Sending data failed");
		dataToSendToLogger.message = "Fail sending temp init success";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
	}
	if(init_status == init_failure)
	{
		dataToSendToLogger.message = "Temperature sensor init failure";
		mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
		pthread_exit(NULL);
	}

	strcpy(dataToSendToMain.source,"temperature");
	dataToSendToLogger.source= "temperature";
	strcpy(dataToSendToLogger.unit,"Celsius");
	while(1)
	{
		/* Do only each 100msec */
		if(measureTemperature)
		{
			int ret = mq_receive(tempQueue,(char*)&dataReceived,sizeof(dataReceived),0);
			if(ret < 0)
			{
				//perror("No requests received");
			}
			else
			{
				sendTempToSocket = true;
			}
			measureTemperature = false;
			dataToSendToMain.messageType = update;
			dataToSendToMain.status = 1;
			
			dataToSendToLogger.status = 1;
			
			/* Get the temperature values in Celsius */
			temperature = read_temperature(i2c_file_des, TEMP_REG_ADDR);
			if(temperature == 10000)
			{
				printf("Temperature sensor pulled out\nExiting Temperature Sensor Task\n");
				dataToSendToLogger.logLevel = alert;
				dataToSendToLogger.status = fail;
				dataToSendToLogger.source= "temperature";
				dataToSendToLogger.message = "Read from sensor failed";
				mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
				gpio_write_value(53,1);	
				close(i2c_file_des);
				mq_close(tempToMain);
				mq_close(tempToLogger);
				mq_close(tempQueue);
				mq_close(socketQueue);
				mq_unlink(MAINQUEUENAME);
				mq_unlink(TEMPQUEUENAME);
				mq_unlink(SOCKETQUEUENAME);
				mq_unlink(LOGQUEUENAME);
				timer_delete(tempTimer);
				pthread_exit(NULL);
			}
			dataToSendToLogger.logLevel = info;
			dataToSendToLogger.value = temperature;

			/* Send heartbeat message to main */
			mq_send(tempToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);
			/* Send temperature values to logger */
			mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
			
			/* Send temperature values to socket only if it is requested. */
			if(sendTempToSocket)
			{
				sendTempToSocket = false;
				dataToSendToSocket.source = "temperature";
				if(strcmp(dataReceived.unit,"Celsius") == 0)
				{
					temperature = convert_to_unit(temperature,Celsius);
				}
				else if(strcmp(dataReceived.unit,"Fahrenheit") == 0)
				{
					temperature = convert_to_unit(temperature,Fahrenheit);
				}
				else if(strcmp(dataReceived.unit,"Kelvin") == 0)
				{
					temperature = convert_to_unit(temperature,Kelvin);
				}
				dataToSendToSocket.value  = temperature;
				int ret = mq_send(socketQueue, (char*)&dataToSendToSocket, sizeof(dataToSendToSocket),0);
				if (ret < 0)
				{
					perror("Temperature sending to socket failed");
					dataToSendToLogger.logLevel = alert;
					dataToSendToLogger.status = fail;
					dataToSendToLogger.source= "temperature";
					strcpy(dataToSendToLogger.message,"Sending temp to socket failed");
					mq_send(tempToLogger,(char*)&dataToSendToLogger,sizeof(logStruct),0);
				}
			}
			if(exitThread)
			{
				break;
			}
		}
		int ret = poll(&fds, 1, 1);
		if(ret > 0)
		{
			gpio_state = gpio_read_val_with_fd(gpio_val_fd);
			if(gpio_state == 0x01)
				printf("Temperature is below 26C\n");
			else
				printf("Temperature rise above 30C\n");
		}
	}
	printf("_____________________Temperature task exiting_______________________\n");
	/* Cleanup all initializations */
	close(i2c_file_des);
	mq_close(tempToMain);
	mq_close(tempToLogger);
	mq_close(tempQueue);
	mq_close(socketQueue);
	mq_unlink(MAINQUEUENAME);
	mq_unlink(TEMPQUEUENAME);
	mq_unlink(SOCKETQUEUENAME);
	mq_unlink(LOGQUEUENAME);
	timer_delete(tempTimer);
    pthread_exit(NULL);
}


/********************************************************************************
* Function name:- lightSensorRoutine                                            *
* Description:- This function is used by a light sensor thread for execution of *
*               light sensor task.                                              *
* @param:- void* (data from main)                                               *
* @return:- static void* (pthread exit value)                                   *
********************************************************************************/
static void* lightSensorRoutine(void *dataObj)
{
    mainStruct dataToSendToMain;
	logStruct dataTOSendTOLogger;
	lightStruct dataReceivedFromMain;
	socketStruct dataToSendToSocket;
	static state presentState, prevState;
    presentState=init;
    prevState=init;
    
	int fd;
	bool sendLightToSocket=false;

	strcpy(dataToSendToMain.source,"light");
	dataToSendToMain.messageType = update;
	dataToSendToMain.status=init_success;

	dataTOSendTOLogger.source="main";
	dataTOSendTOLogger.logLevel=alert;
	dataTOSendTOLogger.status=fail;

	/* Initialize gpio LED pin as output */
	gpio_init(54, out);

	/* Setting up queues */
    mqd_t lightToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(dataToSendToMain));
	mqd_t lightToLogger = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));
	mqd_t lightQueue = mqueue_init(LIGHTQUEUENAME,LIGHT_QUEUE_SIZE,sizeof(lightStruct));
	mqd_t lightToSocket = mqueue_init(SOCKETQUEUENAME,SOCKET_QUEUE_SIZE,sizeof(socketStruct));
    
	if(lightToMain < 0)
    {
        perror("Light queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Light To Main queue creation failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
    }
    if(lightToLogger<0)
	{
		perror("LightToLogger queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="LightToLogger queue creation failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}

	if(lightQueue<0)
	{
		perror("LightQueue queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="LightQueue queue creation failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}
	
	if(lightToSocket<0)
	{
		perror("LightToSocket Queue queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="LightToSocket Queue queue creation failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}

	/* Initialising I2C */
	fd=myi2cInit(slaveAddFloat);
	int ret = lightSensorWrite(fd,CNTRLREG,0x03,2); //writing to control register.
	if(ret < 0)
	{
		dataTOSendTOLogger.message="Writing to control register failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		gpio_write_value(54,high);	
	}	

	/* Initialising Timer */
    struct sigevent lightEvent;	
	struct sigaction lightAction;
	struct itimerspec lightTimerSpec;
	timer_t lightTimer;

	/* Assigning signal handler to a signal */
	lightAction.sa_flags = SA_SIGINFO;
	lightAction.sa_sigaction = signal_handler;	

	
	if(sigaction(SIGRTMIN + 5 , &lightAction, NULL)<0)
	{
		perror("Light sensor, timer handler initialisation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Light sensor, timer handler initialisation failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		
	}	
	
	/* Setting up timer */
	lightEvent.sigev_notify = SIGEV_SIGNAL;
    lightEvent.sigev_signo = SIGRTMIN + 5;
    lightEvent.sigev_value.sival_ptr = &lightTimer;

	if((timer_create(CLOCK_REALTIME, &lightEvent, &lightTimer)) < 0)
	{
		perror("Timer creation failed for temperature task");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Timer creation failed for temperature task";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		//exit(1);
	}

	/* Start the timer */
	lightTimerSpec.it_interval.tv_nsec = 100000000; //To get the value after every 100 ms
	lightTimerSpec.it_interval.tv_sec  = 0;
	lightTimerSpec.it_value.tv_nsec    = 100000000;
	lightTimerSpec.it_value.tv_sec     = 0;

	if(timer_settime(lightTimer,0,&lightTimerSpec,NULL)<0)
	{
		perror("Light sensor timer settime failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Light sensor timer settime failed";
		mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		//exit(1);
	}
 
	/*Sending initialisation success/fail message*/
	mq_send(lightToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);

 	int y=0;
 	float lux=0;
 	while(1)
 	{
		if(measureLight)
		{
			if(mq_receive(lightQueue,(char*)&dataReceivedFromMain,sizeof(lightStruct),0)>-1)
			{
				sendLightToSocket=true;
			}
			measureLight=false;
	
			//take the light sensor value and send it to logger task
			dataToSendToMain.messageType = update;
			dataToSendToMain.status = success;	
		
			dataTOSendTOLogger.source= "light";
			dataTOSendTOLogger.logLevel=alert;
			dataTOSendTOLogger.status=fail;
	
			lux=luxCalc(fd);
			if(lux < 0)
			{
				printf("Light sensor pulled out\n Exiting Light task\n");
				dataTOSendTOLogger.message = "Failed reading the light value";
				mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(dataTOSendTOLogger),0);
				gpio_write_value(54,high);
				close(fd);
				mq_close(lightToMain);
				mq_close(lightToLogger);
				mq_close(lightQueue);
				mq_close(lightToSocket);
		
				mq_unlink(MAINQUEUENAME);
				mq_unlink(LIGHTQUEUENAME);
				mq_unlink(SOCKETQUEUENAME);
				mq_unlink(LOGQUEUENAME);
				timer_delete(lightTimer);
				pthread_exit(NULL);
				}
			
			if(lux>50)
	        {
	        	presentState=light;
	            if(prevState!=light)
	            {
	                printf("State changed to Light\n");
					dataTOSendTOLogger.message="State changed to Light";
	                prevState=presentState;
					mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(dataTOSendTOLogger),0);
	            }
	        }
	        else
	        {
	        	presentState=dark;
	        	if(prevState!=dark)
	            {
	                printf("State changed to Dark\n");
					dataTOSendTOLogger.message="State changed to Dark";
		                prevState=presentState;
					mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(dataTOSendTOLogger),0);
	            }
	        }

			dataTOSendTOLogger.status = success;
			dataTOSendTOLogger.source= "light";
			dataTOSendTOLogger.value = lux;
			dataTOSendTOLogger.logLevel=info;
			int ret=mq_send(lightToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);
			int ret1=mq_send(lightToLogger,(char*)&dataTOSendTOLogger,sizeof(dataTOSendTOLogger),0);
			if(ret1==-1) perror("sending data to log queue failed");
			if(sendLightToSocket)
			{
				sendLightToSocket=false;
				dataToSendToSocket.source="light";
				dataToSendToSocket.value=lux;
				if(mq_send(lightToSocket,(char*)&dataToSendToSocket,sizeof(socketStruct),0)<0)
				{
					perror("sending data to socket failed\n");
				}
			}
			y++;
		}
	    if(exitThread)
		{
			break;
		}
	}

 
	printf("_____________________Light task exiting_______________________\n");
	close(fd);
	mq_close(lightToMain);
	mq_close(lightToLogger);
	mq_close(lightQueue);
	mq_close(lightToSocket);
	
	mq_unlink(MAINQUEUENAME);
	mq_unlink(LIGHTQUEUENAME);
	mq_unlink(SOCKETQUEUENAME);
	mq_unlink(LOGQUEUENAME);
	timer_delete(lightTimer);
	pthread_exit(NULL);
}


/*********************************************************************************
* Function name:- socketRoutine                                                  *
* Description:- This function is used by a socket sensor thread for execution of *
*               socket sensor task.                                              *
* @param:- void* (data from main)                                                *
* @return:- static void* (pthread exit value)                                    *
*********************************************************************************/
static void* socketRoutine(void *dataObj)
{
    printf("Entered socketRoutine\n");
	int serverSocket,clientConnected,len;
	struct sockaddr_in clientAddr,serverAddr;
	struct hostent *ptr;
	int n=0; 
	bool receive = false;
	
	mainStruct dataToSendToMain;
	logStruct dataTOSendTOLogger;
	socketStruct dataReceivedFromSensors;

	dataToSendToMain.status=init_success;
	strcpy(dataToSendToMain.source,"socket");
	dataToSendToMain.messageType = update;

	dataTOSendTOLogger.source="socket";
	dataTOSendTOLogger.logLevel=alert;
	dataTOSendTOLogger.status=fail;

	mqd_t socketToMain = mqueue_init(MAINQUEUENAME,MAIN_QUEUE_SIZE,sizeof(mainStruct));
	mqd_t socketToLogger = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));
	struct mq_attr queue_attr1;
	queue_attr1.mq_maxmsg  = SOCKET_QUEUE_SIZE;
    queue_attr1.mq_msgsize = sizeof(socketStruct);
	mqd_t sensorToSocket = mq_open(SOCKETQUEUENAME, O_CREAT | O_RDWR, 0666, &queue_attr1);
	
	if(socketToMain < 0)
    {
        perror("socketTomain queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="socketTomain queue creation failed";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
    }
    if(socketToLogger<0)
	{
		perror("socketToLogger queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="socketToLogger queue creation failed";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}

	 if(sensorToSocket<0)
	{
		perror("sensorToSocket queue creation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="sensorToSocket queue creation failed";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		
	}
 
	/*Initialising Timer*/
    struct sigevent socketEvent;	
	struct sigaction socketAction;
	struct itimerspec socketTimerSpec;
	timer_t socketTimer;

	socketAction.sa_flags = SA_SIGINFO;
	socketAction.sa_sigaction = signal_handler;		
	
	if(sigaction(SIGRTMIN + 7 , &socketAction, NULL)<0)
	{
		perror("Socket, timer handler initialisation failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Socket, timer handler initialisation failed";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}	
	
	socketEvent.sigev_notify = SIGEV_SIGNAL;
    socketEvent.sigev_signo = SIGRTMIN + 7;
    socketEvent.sigev_value.sival_ptr = &socketTimer;

	/* Creating timer */
	if((timer_create(CLOCK_REALTIME, &socketEvent, &socketTimer)) < 0)
	{
		perror("Timer creation failed for sensor task");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Timer creation failed for sensor task";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}

	socketTimerSpec.it_interval.tv_nsec = 500000000; //To get the heart beat value after every 1 s
	socketTimerSpec.it_interval.tv_sec  = 0;//1;
	socketTimerSpec.it_value.tv_nsec    = 500000000;
	socketTimerSpec.it_value.tv_sec     = 0;//1;

	/* Starting timer */
	if(timer_settime(socketTimer,0,&socketTimerSpec,NULL)<0)
	{
		perror("Socket task timer settime failed");
		dataToSendToMain.status=init_failure;
		dataTOSendTOLogger.message="Socket task timer settime failed";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
		//exit(1);
	}
	
	/*Setting up server socket*/
	serverSocket=socket(AF_INET, SOCK_STREAM, 0);

	memset((char*)&serverAddr,0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(10000);

	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(serverSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr)) == -1)
	{
		printf("Bind Failure\n");
		dataToSendToMain.status=init_failure;		
		dataTOSendTOLogger.message="Bind Failure";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}
	else
	{
		printf("Bind Success:<%u>\n", serverSocket);
		dataTOSendTOLogger.status=success;
		dataTOSendTOLogger.message="Bind Success";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}

	/*Sending init success/fail message to main*/
	mq_send(socketToMain,(char*)&dataToSendToMain,sizeof(mainStruct),0);

	listen(serverSocket,5);
	len=sizeof(struct sockaddr_in);

	printf("----------> Calling accept system call.\n");
	clientConnected=accept(serverSocket,(struct sockaddr*)&clientAddr,&len);
	if (clientConnected !=-1)
	{
		printf("Connection accepted:<%u>\n", clientConnected);
		dataTOSendTOLogger.status=success;
		dataTOSendTOLogger.message="Connection accepted";
		mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	}
	
	int input;
	const int errval=-500;

	while(1)
	{
		if(receive)
		{
			receive = false;
			int ret = mq_receive(sensorToSocket,(char*)&dataReceivedFromSensors,sizeof(socketStruct),0);
			if(ret > -1)
			{
				printf("Data received by server queue from %s\n",dataReceivedFromSensors.source);
			
				if(dataReceivedFromSensors.source=="temperature")
				{
					printf("Temperature readings received from Temp task to socket server\n");
					if(send(clientConnected, (void*)&dataReceivedFromSensors.value,sizeof(dataReceivedFromSensors.value),0)!=sizeof(dataReceivedFromSensors.value))
						{
						perror("Sending temp value to client failed\n");
						dataTOSendTOLogger.status=fail;
						dataTOSendTOLogger.message="Sending temp value to client failed";
						mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
					}
				}
				else if(dataReceivedFromSensors.source=="light")
				{
					printf("Light request received in server. Sending data to client\n");
					if(send(clientConnected, (void*)&dataReceivedFromSensors.value,sizeof(dataReceivedFromSensors.value),0)!=sizeof(dataReceivedFromSensors.value))
					{
						perror("Sending light value to client failed\n");
						dataTOSendTOLogger.status=fail;
						dataTOSendTOLogger.message="Sending light value to client failed";
						mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
					}
				}
				else
				{
					strcpy(dataTOSendTOLogger.message,dataReceivedFromSensors.message);
					strcpy(dataTOSendTOLogger.source,"socket");
					dataTOSendTOLogger.status=fail;
					dataTOSendTOLogger.logLevel=alert;
					mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
					if(send(clientConnected, (void*)&errval,sizeof(errval),0)!=sizeof(errval))
					{
						perror("Sending error value to client failed\n");
						dataTOSendTOLogger.status=fail;
						dataTOSendTOLogger.message="Sending error value to client failed";
						mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
					}
				}
			}
		}
		int rb=read(clientConnected,&input, sizeof(input));
		if(rb==sizeof(input))
		{
			receive = true;
			printf("Message received from client is %d\n",input);
			strcpy(dataToSendToMain.source,"socket");
			dataTOSendTOLogger.status=success;
			dataTOSendTOLogger.logLevel=alert;
			dataTOSendTOLogger.message="Request message received from client";
			mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);

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
			mq_send(socketToMain,(char *) &dataToSendToMain,sizeof(mainStruct),0);
		} 
		if(exitThread)
		{
			break;
		}
	}
	printf("_____________________Socket task exiting_______________________\n");

	mq_close(socketToMain);
	mq_close(socketToLogger);
	mq_close(sensorToSocket);
	mq_unlink(MAINQUEUENAME);
	mq_unlink(LOGQUEUENAME);
	mq_unlink(SOCKETQUEUENAME);
	close(serverSocket);

	dataTOSendTOLogger.logLevel=alert;
	dataTOSendTOLogger.message="Server Socket Closed !!";
	mq_send(socketToLogger,(char*)&dataTOSendTOLogger,sizeof(logStruct),0);
	
	printf("\nServer Socket Closed !!\n");
	pthread_exit(NULL);
}


/*********************************************************************************
* Function name:- loggerRoutine                                                  *
* Description:- This function is used by a logger thread.                        *
* @param:- void* (data from main)                                                *
* @return:- static void* (pthread exit value)                                    *
*********************************************************************************/
static void* loggerRoutine(void *dataObj)
{
    mainStruct dataToSend;
	logStruct dataToReceive;
	
	mainInfoToOthers *obj1;
	obj1=malloc(sizeof(mainInfoToOthers));
	obj1=(mainInfoToOthers *)dataObj;

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

	loggerTimerSpec.it_interval.tv_nsec = 0;
	loggerTimerSpec.it_interval.tv_sec  = 2;
	loggerTimerSpec.it_value.tv_nsec    = 0;
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
        perror("logger queue creation failed");
    }

	mqd_t loggerQueue = mqueue_init(LOGQUEUENAME,LOG_QUEUE_SIZE,sizeof(logStruct));	
	if(loggerQueue < 0)
    {
        perror("logger queue creation failed");
    }	

	mq_send(loggerToMain,(char*)&dataToSend,sizeof(mainStruct),0);
	int y=0;
 	while(1)
 	{
		if(loggerHeartBeat)
		{
			loggerHeartBeat=false;
			dataToSend.messageType = update;
			dataToSend.status = success;	
			mq_send(loggerToMain,(char*)&dataToSend,sizeof(mainStruct),0);	
			y++;
		}
	 	else
		{
			int ret= mq_receive(loggerQueue, (char*)&dataToReceive, sizeof(logStruct),0);
			if(ret!=-1)
			{
			 	logToFile(obj1->logFileName, dataToReceive);	
			}
		}
		if(exitThread)
		{
			break;
		} 
	}
	printf("_____________________Logger task exiting_______________________\n");
	mq_close(loggerToMain);
	mq_close(loggerQueue);
	mq_unlink(MAINQUEUENAME);
	mq_unlink(LOGQUEUENAME);
    timer_delete(loggerTimer);
	pthread_exit(NULL);
}

uint8_t isAlive = 0x0F;
int count;
int main(int argc, char *argv[])
{
	printf("Inside main task\n");
	count=1;
    mainStruct dataToReceive;
	tempStruct requestForTemp;
	socketStruct responseToSocket;
	lightStruct requestForLight;
	logStruct dataToLog;
	mainInfoToOthers dataObj;

	if(argc != 2)
	{
		printf("Try execution in below syntax\n");
		printf("Execute in format -> ./maintask <log file name>\n");
		return 0;
	}

	/* I2C initialisations */
	fd=myi2cInit(slaveAddFloat);
	fd1=temp_i2c_init(DEFAULT_SLAVE_ADDRESS);
	printf("fd in maintask is %d\n",fd);

	/* Initialize logger queue */
	mqd_t logQueue = mqueue_init(LOGQUEUENAME, LOG_QUEUE_SIZE, sizeof(logStruct));
    if(logQueue < 0)
    {
        perror("Main queue creation failed");
    }

	/*Built in self test calls*/
	int lightBIST=lightSensorBIST(fd);
	if(lightBIST==-1)
	{
		dataToLog.source = "main";
		dataToLog.message = "Light sensor BIST failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		mq_close(logQueue);
		mq_unlink(LOGQUEUENAME);
		printf("Light sensor BIST failed\n");
		return 0;
	}
	else
	{
		dataToLog.source = "main";
		dataToLog.message = "Light sensor BIST passed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		printf("Light sensor BIST passed\n");
	}

	int tempBIST=tempSensorBIST(fd1);
	if(tempBIST==-1)
	{
		dataToLog.source = "main";
		dataToLog.message = "Temp sensor BIST failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		mq_close(logQueue);
		mq_unlink(LOGQUEUENAME);
		printf("Temp sensor BIST failed\n");
		return 0;	
	}
	else
	{
		dataToLog.source = "main";
		dataToLog.message = "Temp sensor BIST passed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		printf("Temp sensor BIST passed\n");	
	}
	
	dataObj.logFileName=malloc(20);
	strcpy(dataObj.logFileName,argv[1]);
	printf("Received file name is %s\n",dataObj.logFileName);

	dataToLog.source = "main";

	//Creating queues for Inter Process Communication
    mqd_t mainQueue = mqueue_init(MAINQUEUENAME, MAIN_QUEUE_SIZE, sizeof(mainStruct));
    if(mainQueue < 0)
    {
		dataToLog.message = "Main queue creation failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("Main queue creation failed");
    }

    mqd_t tempQueue = mqueue_init(TEMPQUEUENAME, TEMP_QUEUE_SIZE, sizeof(tempStruct));
    if(tempQueue < 0)
    {
		dataToLog.message = "Temp queue creation failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("Temp queue creation failed");
    }

    mqd_t lightQueue = mqueue_init(LIGHTQUEUENAME, LIGHT_QUEUE_SIZE, sizeof(lightStruct));
    if(lightQueue < 0)
    {
		dataToLog.message = "Light queue init failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("Light queue init failed");
    }
	
	mqd_t socketQueue = mqueue_init(SOCKETQUEUENAME, SOCKET_QUEUE_SIZE, sizeof(socketStruct));
	if(socketQueue < 0)
	{
		dataToLog.message = "Socket queue creation failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Socket queue creation failed");
    }

    printf("Main thread created with PID: %d and TID: %d\n",getpid(),(pid_t)syscall(SYS_gettid));

    /*Creating Temperature Sensor Thread */
    if(pthread_create(&tempSensorTask,NULL,&tempSensorRoutine,(void *)&dataObj)!=0)
    {
		dataToLog.message = "tempSensorTask create failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("tempSensorTask create failed");
    }
    
    /*Creating Light Sensor Thread */
    if(pthread_create(&lightSensorTask,NULL,&lightSensorRoutine,(void *)&dataObj)!=0)
    {
		dataToLog.message = "lightSensorTask create failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("lightSensorTask create failed");
    }

    /*Creating Logger Thread */
    if(pthread_create(&loggerTask,NULL,&loggerRoutine,(void *)&dataObj)!=0)
    {
		dataToLog.message = "loggerTask create failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("loggerTask create failed");
    }

    /*Creating Socket Thread */
    if(pthread_create(&socketTask,NULL,&socketRoutine,(void *)&dataObj)!=0)
    {
		dataToLog.message = "socketTask create failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
        perror("socketTask create failed");
    }

	struct sigevent sigevTemp, sigevLight, sigevLog, sigevSocket;
	struct itimerspec timerConfigTemp, timerConfigLight, timerConfigLog, timerConfigSocket;
	timer_t timerTemp, timerLight, timerSocket, timerLog;
	struct sigaction sigact;

	//Assigning signal handlers for timeout signals
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

	if((sigaction(SIGINT, &sigact, NULL))<0)
	{
		perror("Failed setting signal handler");
	}
	
	//Creating 4 timer for monoring 4 created tasks.
	sigevTemp.sigev_notify            = SIGEV_SIGNAL;
	sigevTemp.sigev_signo             = SIGRTMIN;
	sigevTemp.sigev_value.sival_ptr   = &timerTemp;
	
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
		dataToLog.message = "Temp Timer setup failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Temp Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevLight, &timerLight)) < 0)
	{
		dataToLog.message = "Light Timer setup failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Light Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevLog, &timerLog)) < 0)
	{
		dataToLog.message = "Log Timer setup failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Log Timer setup failed");
		exit(1);
	}
	if((timer_create(CLOCK_REALTIME, &sigevSocket, &timerSocket)) < 0)
	{
		dataToLog.message = "Socket Timer setup failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Socket Timer setup failed");
		exit(1);
	}

	//Setiing supervisory timeout for each tasks.
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
	timerConfigLog.it_value.tv_sec     = 10; 

	timerConfigSocket.it_interval.tv_nsec = 0;
	timerConfigSocket.it_interval.tv_sec  = 0;
	timerConfigSocket.it_value.tv_nsec    = 0;
	timerConfigSocket.it_value.tv_sec     = 15;  

	if((timer_settime(timerTemp, 0, &timerConfigTemp, NULL)) < 0)
	{
		dataToLog.message = "Temp Timer set failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Temp Timer set failed");
		exit(1);
	}
	if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
	{
		dataToLog.message = "Light Timer set failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Light Timer set failed");
		exit(1);
	}
	if((timer_settime(timerLog, 0, &timerConfigLog, NULL)) < 0)
	{
		dataToLog.message = "Log Timer set failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Log Timer set failed");
		exit(1);
	}
	if((timer_settime(timerSocket, 0, &timerConfigSocket, NULL)) < 0)
	{
		dataToLog.message = "Socket Timer set failed";
		mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
		perror("Socket Timer set failed");
		exit(1);
	}
	
	// Check if the initialization of all the tasks is done.
	int task_up = 0, retries = 0;
	while(task_up != 4)
	{
		int ret=mq_receive(mainQueue, (char*)&dataToReceive, sizeof(mainStruct),0);
		if (ret > -1 && dataToReceive.status == init_success)
		{
			printf("---%s Task is up and running---\n",dataToReceive.source);
			task_up++;	
		}
		else if(ret > -1 && dataToReceive.status == init_failure)
		{
			retries++;
		}
		if(retries > 10)
		{
			dataToLog.message = "All the tasks are not up.";
			mq_send(logQueue, (char*)&dataToLog, sizeof(dataToLog),0);
			printf("All tasks are not up");
			return 0;
		}
	}
	

    while(1)
    {
        int ret=mq_receive(mainQueue, (char*)&dataToReceive, sizeof(mainStruct),0);
        if(ret>-1)
        {
			//Heart beat message received from temperature task
			if((strcmp(dataToReceive.source,"temperature"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					isAlive = isAlive | TEMPERATURE_TASK;
					break;
				case success:
					if((timer_settime(timerTemp, 0, &timerConfigTemp, NULL)) < 0)
					{
						perror("Temp Timer set failed");
					}
					break;
				case fail:
				case init_failure:
					isAlive = isAlive & ~TEMPERATURE_TASK;
					break;
				}
			}
		
			//Heart beat message received from light task
			else if((strcmp(dataToReceive.source,"light"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					isAlive = isAlive | LIGHT_TASK;
					break;
				case success:
					if((timer_settime(timerLight, 0, &timerConfigLight, NULL)) < 0)
					{
						perror("Light Timer set failed");
					}
					break;
				case fail:
				case init_failure:
					isAlive = isAlive & ~LIGHT_TASK;
					break;
				}
			}

			//Heart beat message received from logger task
			else if((strcmp(dataToReceive.source,"logger"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					isAlive = isAlive | LOGGER_TASK;
					break;
				case success:
					if((timer_settime(timerLog, 0, &timerConfigLog, NULL)) < 0)
					{
						perror("Logger Timer set failed");
					}
					break;
				case fail:
				case init_failure:
					isAlive = isAlive & ~LOGGER_TASK;
					break;
				}
			}

			//Heart beat message received from socket task
			else if((strcmp(dataToReceive.source,"socket"))==0 && dataToReceive.messageType == update)
			{
				switch(dataToReceive.status)
				{
				case init_success:
					isAlive = isAlive | SOCKET_TASK;
					break;
				case success:
					if((timer_settime(timerSocket, 0, &timerConfigSocket, NULL)) < 0)
					{
						perror("Socket Timer set failed");
					}
					break;
				case fail:
				case init_failure:
					isAlive = isAlive & ~SOCKET_TASK;
					break;
				}
			}

			//Request message is recevied from socket server for getting temperature or light values
			else if((strcmp(dataToReceive.source,"socket"))==0 && dataToReceive.messageType == request)
			{
				if(strcmp(dataToReceive.unit,"Celsius")==0 || strcmp(dataToReceive.unit,"Fahrenheit")==0 || strcmp(dataToReceive.unit,"Kelvin")==0)
				{
					if(isAlive & TEMPERATURE_TASK)
					{
						printf("\nTemp task is alive\n");
						printf("Sending request to temp sensor task to send data to socket task\n");
						strcpy(requestForTemp.source, "main");
						strcpy(requestForTemp.unit, dataToReceive.unit);
						int noOfBytesSent = mq_send(tempQueue, (char*)&requestForTemp, sizeof(requestForTemp), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending request failed");
						}
					}
					else
					{
						printf("Temp sensor task is not active\n");
						strcpy(responseToSocket.source, "main");
						strcpy(responseToSocket.message, "Temp Sensor Task not active");
						int noOfBytesSent = mq_send(socketQueue, (char*)&responseToSocket, sizeof(responseToSocket), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending response failed");
						}
					}
				}
				else
				{
					if(isAlive & LIGHT_TASK)
					{
						printf("\nLight task is alive\n");
						printf("Sending request to light sensor task to send data to socket task\n");
						strcpy(requestForLight.source, "main");
						int noOfBytesSent = mq_send(lightQueue, (char*)&requestForLight, sizeof(requestForLight), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending request failed");
						}
					}
					else
					{
						printf("Light sensor task is not active\n");
						strcpy(responseToSocket.source, "main");
						strcpy(responseToSocket.message,"Light sensor task not active");
						int noOfBytesSent = mq_send(socketQueue, (char*)&responseToSocket, sizeof(responseToSocket), 0);
						if(noOfBytesSent < 0)
						{
							perror("Sending response failed");
						}
					}
				}
			}
		}
		if(!isAlive)
		{
			break;
		}	
		
    }
	printf("_____________________Main task exiting_______________________\n");
    mq_close(mainQueue);
	mq_close(tempQueue);
	mq_close(lightQueue);
	mq_close(socketQueue);
	
	mq_unlink(MAINQUEUENAME);
	mq_unlink(TEMPQUEUENAME);
	mq_unlink(LIGHTQUEUENAME);
	mq_unlink(SOCKETQUEUENAME);

    pthread_join(tempSensorTask,NULL);
    pthread_join(lightSensorTask,NULL);
    pthread_join(loggerTask,NULL);
	pthread_cancel(socketTask);
    pthread_join(socketTask,NULL);
    return 0;
}


static void signal_handler(int sig, siginfo_t *si, void *uc)
{
	switch(sig)
	{
	case 2:
		printf("SIGINT signal is received\n ----------> Exiting thread <---------\n");
		exitThread = true;		
		break;
	case 34:
		// Siganl indicating temperature sensor timeout i.e. Temperature task failed
		printf("Temperature task timeout\n");
		isAlive &= ~TEMPERATURE_TASK;
		break;
	case 35:
		// Signal indicating light sensor timeout i.e. Light task failed
		printf("Light Task Timeout\n");
		isAlive &= ~LIGHT_TASK;
		break;
	case 36:
		// Signal indicating logger timeout i.e. Logger task failed
		printf("Logger Task Timeout\n");
		isAlive &= ~LOGGER_TASK;
		break;
	case 37:
		// Signal indicating socket sensor timeout i.e. Socket task failed
		isAlive &= ~SOCKET_TASK;
		break;
	case 38:
		// Signal instructing to take temperature sensor reading and send heartbeat
		measureTemperature = true;
		break;
	case 39:
		// Signal instructing to take light sensor reading and send heartbeat
		measureLight = true;
		break;
	case 40:
		// Signal instructing logger task to send heartbeat to main indicating it is alive
		loggerHeartBeat = true;
		break;
	case 
		socketHeartBeat = true;
		break;
	}
}
