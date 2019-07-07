/************************************************************************************************
* File name   : bist.c                                                                          *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function definition used for built in self test.                            *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include "bist.h"
#include <stdio.h>
#include "temp_i2c.h"

int lightSensorBIST(int fileDesc)
{
    uint8_t* rb= malloc(6);
    rb=lightSensorRead(fileDesc,IDREG,1);
	if(rb == NULL)
	{
		return -1;
	}
    if(*rb!=0x50)
    {
        return -1;
    }
return 0;
}


int tempSensorBIST(int fileDesc)
{
    uint16_t retVal=temp_i2c_read_from_reg(fileDesc, CONFIG_REG_ADDR);
	if(retVal == 10000)
		return -1;
    if(retVal!=DEFAULT_CONFIG)
        return -1;
    return 0;
}

