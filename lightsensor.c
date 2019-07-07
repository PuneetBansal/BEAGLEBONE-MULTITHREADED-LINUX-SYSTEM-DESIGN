/************************************************************************************************
* File name   : lightsonsor.c                                                                   *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function definition used for communication with light sensor.               *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include<stdio.h>
#include"myi2c.h"
#include<stdint.h>
#include"lightsensor.h"

uint16_t data0Val;
uint16_t data1Val;
float lux;
float ch1ch0Ratio;


uint8_t* lightSensorRead(int fileDescrip, uint8_t regAdd, uint8_t len)
{
	uint8_t* readBuffer= malloc(10);
	uint8_t writeBuffer[10];
	uint8_t cmdReg=0;
	int ret;
	if(len==1)
	{
	    cmdReg=CMDBYTE;
	}
	else if(len==2)
	{
	    cmdReg=CMDWORD;
	}
	else if(len<=0 || len>2)
	{
    	exit(1);
	}
	*writeBuffer=(cmdReg | regAdd);
	ret=myi2cWrite(fileDescrip,writeBuffer,1);
	readBuffer=myi2cRead(fileDescrip,len);
	if(readBuffer == NULL)
	{
		perror("Read failed");
		return NULL;
	}
	return readBuffer;
}



int lightSensorWrite(int fileDescrip,uint8_t regAdd, uint16_t data, uint8_t len)
{
	uint8_t writeBuffer[10];
	int ret;
	
	uint8_t cmdReg=0;
	if(len==1)
	{
	    cmdReg=CMDBYTE;
	}
	else if(len==2)
	{
	    cmdReg=CMDWORD;
	}
	else if(len<=0 || len>2)
	{
	    printf("Invalid len");
	    exit(1);
	}
	writeBuffer[0]=(cmdReg | regAdd);
	writeBuffer[1]=data;
	ret=myi2cWrite(fileDescrip,writeBuffer,len);
	if(ret < 0)
	{
		perror("Write failed");
		return -1;
	}
	return 0;
}


float luxCalc(int fd)
{
    data0Val=0;
    data1Val=0;
    ch1ch0Ratio=0;
    lux=0; 
        
    uint8_t* readBuffer=malloc(2);
    readBuffer=lightSensorRead(fd,DATA0LOW,2);
	if(readBuffer == NULL)
	{
		perror("Read failed");
		return -1;
	}
    
    data0Val=readBuffer[1]<<8;
    data0Val |= readBuffer[0];

    readBuffer=lightSensorRead(fd,DATA1LOW,2);
	if(readBuffer == NULL)
	{
		perror("Read failed");
		return -1;
	}

    data1Val=readBuffer[1]<<8;
    data1Val |= readBuffer[0];
  
    float temp0=data0Val;
    float temp1=data1Val;
    
    
    ch1ch0Ratio=temp1/temp0;
    

   	if(ch1ch0Ratio>0 && ch1ch0Ratio<=0.50)
   	{
   		lux= ( ( (0.0304)*data0Val ) - (0.062 * data1Val * pow(ch1ch0Ratio,1.4)) );
   	}
	else if(ch1ch0Ratio>0.50 && ch1ch0Ratio<=0.61)	
	{
		lux= ( ( (0.0224)*data0Val ) - (0.031 * data1Val) );
	}
	else if(ch1ch0Ratio>0.61 && ch1ch0Ratio<=0.80)
	{
		lux= ( ( (0.0128)*data0Val ) - (0.0153 * data1Val) );
	}
	else if(ch1ch0Ratio>0.80 && ch1ch0Ratio<=1.30)
	{
		lux= ( ( (0.00146)*data0Val ) - (0.00112 * data1Val) );
	}
	else if(ch1ch0Ratio>1.30)
	{
		lux=0;
	}
	
	return lux;
}

