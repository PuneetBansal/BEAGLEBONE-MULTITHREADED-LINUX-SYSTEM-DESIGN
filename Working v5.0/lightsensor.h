/************************************************************************************************
* File name   : lightsonsor.h                                                                   *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function declaration used for communication with light sensor.              *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "myi2c.h"
#include <math.h>

uint8_t readBuffer[10];

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
#define TIMINGREG 0x01
#define THRESHLOWLOW 0x02
#define THRESHLOWHIGH 0x03
#define THRESHHIGHLOW 0x04
#define THRESHHIGHHIGH 0x05
#define INTCNTL 0X06
#define IDREG 0x0a


/**
 * @name : lightsensorRead
 * 
 * @description: Function to read from the specified i2c registers using i2c.
 * The register address from where data is to be read, is first written via myi2cwrite function, which is basically a wrapper to write to the file 
 * specified by the filedescriptor. After this a read operation of the required number of bytes is performed via myi2cread function. The value received
 * is written on the buffer and returned.
 *
 *  
 * @param1: filedescrip- i2c file descriptor
 * @param2: regadd- light sensor register to read fromm
 * @param3: len- number of bytes that should be read (1/2 bytes)
 * 
 * 
 * @return type: (uint8_t*)- character buffer consisting of the value read from the i2c read function.
 * 
 * */
uint8_t* lightSensorRead(int, uint8_t, uint8_t);


/**
 * @name: lightSensorWrite
 * @description: Writes the specified number of bytes(len) of data (specified in parameters) to the register (specified in parameter) 
 * 
 * @param1: fileDescrip- i2c file descriptor
 * @param2: regAdd- light sensor register to write to 
 * @param3: data: 16bit/8bit data to write to the register.
 * @param4: len: number of bytes tot write (1/2)
 * */
int lightSensorWrite(int fileDescrip,uint8_t, uint16_t, uint8_t);


/**
 * @name: luxCalc
 * @desc: reads from adc channel 0 and adc channel 1 using lightSensorRead function. Does the necessary computations to calculate LUX and
 * returns the lux value in float to the user.
 * 
 * @param1- i2c file descriptor
 * 
 * @return type:  float - lux value in float.* 
 * 
 * */
float luxCalc(int);

