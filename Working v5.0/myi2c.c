/************************************************************************************************
* File name   : myi2c.c                                                                         *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function definition for reading and writing to i2c device                   *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
// ref:https://elinux.org/Interfacing_with_I2C_Devices

#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "myi2c.h"

int myi2cInit(uint8_t slaveAdd)
{
char fileName[50];
int fileDescrip;
sprintf(fileName,"/dev/i2c-2");
fileDescrip=open(fileName,O_RDWR);

if(fileDescrip < 0)
{
  perror("Failed to open i2c file");
  exit(1);
}

if(ioctl(fileDescrip, I2C_SLAVE, slaveAdd) < 0) 
{
  perror("Failed to communicate with slave"); //or use errno
  exit(1);
}
return fileDescrip;
}

uint8_t* myi2cRead(int fileDescrip,uint8_t len)
{
static uint8_t readBuffer[2];

pthread_mutex_lock(&lock);
if (read(fileDescrip,readBuffer,len) != len)
{
  perror("Read failed");
  pthread_mutex_unlock(&lock);
  return NULL;
  //check whether we need to return or not.
}
pthread_mutex_unlock(&lock);
//printf("Value of buffer is %x-%x\n",readBuffer[0],readBuffer[1]);
return readBuffer;
}

int myi2cWrite(int fileDescrip, uint8_t writeBuffer[], uint8_t len)
{
pthread_mutex_lock(&lock);
if (write(fileDescrip,writeBuffer,len) != len)
{
pthread_mutex_unlock(&lock);
perror("Write failed");
return -1;
}
pthread_mutex_unlock(&lock);
return 0;

}
