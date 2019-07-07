/************************************************************************************************
* File name   : myi2c.h                                                                         *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function declaration for reading and writing to i2c device                  *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include <stdint.h>
#include <pthread.h>

#define slaveAddFloat 0x39
#define slaveAddGnd 0x29
#define slaveAddVdd 0x49

/*Light Sensor Register addressess*/
#define CMDBYTE 0x80
#define CMDWORD 0xA0
#define CNTRLREG 0x00
#define INTCTL 0x06
#define DATA0LOW 0x0c
#define DATA0HIGH 0x0d
#define DATA1LOW 0x0e
#define DATA1HIGH 0x0f

/**
 * @name: myi2cInit
 * @param1: uint8_t slaveAdd
 * 
 * @description: Opens the /dev/i2c-2 file and designates the slave. 
 * 
 * @return: int- i2c file descriptor.
 * */
int myi2cInit(uint8_t slaveAdd);


/**
 * @name: myi2cRead
 * @param1: fileDescriptor
 * @param2: length of bytes to be read
 * 
 * @description:
 * wrapper around the read system call to read the designated number of bytes from the file. When the length read from file is equal
 * to the length specified by the user, it indicates success. NULL is returned on failure.
 *  
 * @return: uint8_t* - buffer containing the data read from file
 * */
uint8_t* myi2cRead(int, uint8_t);


/**
 * @name:myi2cWrite
 * @aparam1 : filedescriptor
 * @param2 :  data to write 
 * @param3 :  the length of bytes to be written.
 * 
 * @description: 
 * wrapper around the write system call to write the designated number of bytes to the file. When the length written to file is equal
 * to the length specified by the user, it indicates success and returns 0 . 0 is returned on failure.
 * 
 * return: 0 for success and -1 for error.
 * */
int myi2cWrite(int,uint8_t[],uint8_t);

pthread_mutex_t lock;
