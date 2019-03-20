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
if (read(fileDescrip,readBuffer,len) != len)
{
  perror("Read failed");
  return NULL;
  //check whether we need to return or not.
}
return readBuffer;
}

int myi2cWrite(int fileDescrip, uint8_t writeBuffer[], uint8_t len)
{

if (write(fileDescrip,writeBuffer,len) != len)
{
perror("Write failed");
return -1;
}
return 0;

}
