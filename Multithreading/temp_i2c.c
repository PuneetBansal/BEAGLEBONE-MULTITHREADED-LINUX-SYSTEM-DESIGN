/************************************************************************************************
* File name   : temp_i2c.c                                                                      *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for reading and configuring temperature sensor TMP102 for    *
*               getting the temperature values through i2c interface.                           *
* Tools used  : GNU make.                                                                       *
************************************************************************************************/

/************ The standard C libraries included for functionality ************/
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <mqueue.h>

/************* The user library containing required information *************/
#include "temp_i2c.h"
#include "myi2c.h"
//#include "logger.h"


int temp_i2c_init(uint8_t slave_addr)
{
	return myi2cInit(slave_addr);
}


void temp_i2c_write_to_reg(int file_des, uint8_t temp_sens_reg_to_write, int16_t data_to_write)
{
	uint8_t buffer[3];
	buffer[0] = temp_sens_reg_to_write;
	uint16_t config_reg, config_value;

	switch(temp_sens_reg_to_write)
	{
	// If there is write to temperature read only register
	case TEMP_REG_ADDR:
		printf("%s::Not allowed to write to read only register",__func__);
	break;

	// If there is write to config register
	case CONFIG_REG_ADDR:
		config_value = temp_i2c_read_from_reg(file_des, CONFIG_REG_ADDR);
		config_reg = config_value | data_to_write;
		buffer[1] = config_reg >> 8;
		buffer[2] = config_reg << 8;
	break;

	// If there is write operation to Tlow or Thigh register
	case TLOW_REG_ADDR:
	case THIGH_REG_ADDR:
		data_to_write = data_to_write/0.0625;
		buffer[1] = data_to_write >> 4;
		buffer[2] = data_to_write << 4;
	break;
	}
	// Write the buffer values to temp sensor register using i2c
	if(myi2cWrite(file_des, buffer, sizeof(buffer)) < 0)
	{
		printf("%s",__func__);
		perror("Write failed: ");
	}
}


uint16_t temp_i2c_read_from_reg(int file_des, uint8_t temp_sens_to_read_from)
{
	uint8_t* buffer;
	uint16_t reg_val;
	if(myi2cWrite(file_des, &temp_sens_to_read_from, sizeof(temp_sens_to_read_from)) < 0)
	{
		printf("%s\n",__func__);
		perror("Write failed: ");
	}
	buffer = myi2cRead(file_des, 2);
	printf("Value of buffer in %s is %x-%x\n",__func__,buffer[0],buffer[1]);
	return ((uint16_t)buffer[0] << 8 | buffer[1]);
}


float read_temperature(int file_des, uint8_t temp_sens_to_read_from, int temperature_unit)
{	
	int16_t temperature;
	float final_temp;
	
	if(temp_sens_to_read_from == CONFIG_REG_ADDR)
	{
		printf("%s::Config register values are not temperature",__func__);
		exit(1);
	}
	temperature = temp_i2c_read_from_reg(file_des, temp_sens_to_read_from);
	printf("temperature = %x\n",temperature);	
	temperature = temperature >> 4;
	printf("temperature = %x\n",temperature);	
	final_temp = temperature * 0.0625;
	printf("final_temp = %d\n",final_temp);
	switch(temperature_unit)
	{
	case Celsius:
		final_temp = final_temp;
		break;
	case Fahrenheit:
		final_temp = final_temp * 1.8;
		final_temp = final_temp + 32;
		break;
	case Kelvin:
		final_temp = final_temp + 273.5;
		break;	
	}
	return final_temp;
}


/*void *temperature_monitor(void* information)
{
	//write the configurations in temperature sensor
	int file_fd;                                              //get this from the main task
	uint16_t temp_sens_config = MODE_12BIT 
                                | REFRESH_4HZ 
								| SHUTDOWN_MODE_DISABLE
								| COMPARATOR_MODE
								| ALERT_ACTIVE_LOW
								| ALERT_ON_4_FAULT;

	temp_i2c_write_to_reg(file_fd, TLOW_REG_ADDR, TLOW_REG_DEFAULT);
	temp_i2c_write_to_reg(file_fd, THIGH_REG_ADDR, THIGH_REG_DEFAULT);
	temp_i2c_write_to_reg(file_fd, CONFIG_REG_ADDR, temp_sens_config);


	//Initialize the queue for temperature task and logger.
	mqd_t temperature_mqueue, logger_mqueue;
	
	temperature_mqueue = mqueue_init(TEMP_SENS_QUEUE, TEMP_QUEUE_SIZE, sizeof(temp_msg));
	if(temperature_mqueue == (mqd_t) -1)
	{
		perror("Error opening the temperature queue:");
		exit(1);
	}

	logger_mqueue = mqueue_init(LOGGER_QUEUE, LOGGER_QUEUE_SIZE, sizeof(logger_msg));
	if(logger_mqueue == (mqd_t) -1)
	{
		perror("Error opening the logger queue:");
	}

	//setup a timer to fire a signal every short interval to wakeup and take the readings
	//reference: http://man7.org/linux/man-pages/man2/timer_create.2.html 
	timer_t temp_timer_id;
	struct sigevent signal_event;
	struct sigaction temp_action;
	struct itimerspec wakeup_time;

	temp_action.sa_flags     = SA_SIGINFO;
	temp_action.sa_sigaction = signal_handler;
	if((sigaction(SIGNAL_NO, &temp_action, NULL)) < 0)
	{
		perror("Setting signal action failed:");
		exit(1);
	}

	signal_event.sigev_notify          = SIGNAL_NOTIFICATION_METHOD;
	signal_event.sigev_signo           = SIGNAL_NO;
	signal_event.sigev_value.sival_ptr = &temp_timer_id;
	if((timer_create(CLOCK_TO_USE, &signal_event, &temp_timer_id)) < 0)
	{
		perror("Timer creation failed");
		exit(1);
	}

	wakeup_time.it_value.tv_sec     = TIME_IN_NANOSEC / 1000000000;
	wakeup_time.it_value.tv_nsec    = TIME_IN_NANOSEC % 1000000000;
	wakeup_time.it_interval.tv_sec  = TIME_IN_NANOSEC / 1000000000;
	wakeup_time.it_interval.tv_nsec = TIME_IN_NANOSEC % 1000000000;
	
	//take the readings from sensor
	float temperature;
	struct temp_msg *data_received;
	
	temperature = read_temperature(file_fd, TEMP_REG_ADDR, DEFAULT_UNIT);
	printf("Temperature is %f",temperature);
	//if readings are valid then send the heart beat message to the main task.
	if(temperature > -55 && temperature < 128)
	{
		struct logger_msg data_send;
		data_send.source       = "temperature";
		data_send.message_type = success;
		data_send.value        = temperature;
		data_send.unit         = DEFAULT_UNIT;
	
		int ret = mq_send(temperature_mqueue, (char*)&data_send, sizeof(data_send), 0);
		if(ret == -1)
		{
			perror("Send error:");
		}
		//send heart-beat to main, temperature value to logger.
	}
	
	//check if there is message from main task in temp_queue.
	char buffer[100];
	int char_received;	
	char_received = mq_receive(temperature_mqueue, buffer, sizeof(buffer),0) 
	if(char_received < 0)
	{
		perror("Msg not received");
	}
	else
	{
		data_received = (temp_msg*)buffer;
	}
	if((strcmp(data_recevied->source,"socket"))==0)
	{
		temperature = read_temperature(file_fd, TEMP_REG_ADDR, data_received->temperature_unit)
		//write the source as temperature, write the value of temperature and temperature unit
		//open the socket queue and send the message to socket queue	
	}
}

mqd_t mqueue_init(char* queue_name, int queue_size, int message_size)
{
	mqd_t msg_q_des;
	struct mq_attr queue_attr;
	
	queue_attr.mq_maxmsg  = queue_size;
	queue_attr.mq_msgsize = message_size;
	queue_attr.mq_flags   = O_NONBLOCK;
	
	msg_q_des = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &queue_attr);

	return msg_q_des;
}*/
