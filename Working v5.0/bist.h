/************************************************************************************************
* File name   : bist.h                                                                          *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function declarations used for built in self test.                          *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include "lightsensor.h"

/*
 *@name: lightSensorBIST
 *@param: i2c file descriptor
 *@description: reads from the ID register of the light sensor and compares it with the default 
 *               value to make sure sensor is powered on and the I2c communication is active.
 */
int lightSensorBIST(int fileDesc);

/*
 * @name: tempSensorBIST
 * @param: i2c file descriptor
 * @description: reads from the configuration register of the temp sensor and compares it with
 *        the default value to make sure sensor is powered on and the I2c communication is active.
 */
int tempSensorBIST(int fileDesc);
