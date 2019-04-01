#include <stdio.h>
#include "../lightsensor.h"
#include <assert.h>

void main()
{
    int fd;
    int ret;
    uint8_t* rb= malloc(6);
    //uint8_t* rb1= malloc(6);
    fd=myi2cInit(slaveAddFloat);
    printf("file descriptor is %d\n",fd);
    
    

    //lightSensorWrite(fd,CNTRLREG,0x03,2); //writing to control register.
    //printf("Now calling lightSensor Read\n");
    //rb=lightSensorRead(fd,CNTRLREG,1);
    //printf("\nValue read from control register is %d\n",*rb);
	
/*
    printf("\nWriting to the timing register");
    lightSensorWrite(fd,TIMINGREG,0x11,2); //writing to timing register.
    printf("\nReading timing register\n");
    rb=lightSensorRead(fd,TIMINGREG,1);
    printf("rb is pointing to %p\n",rb);
    printf("\nValue read from timing register is %x\n",*rb); */

    printf("Now Reading the id register\n");
    rb=lightSensorRead(fd,IDREG,1);
    printf("Value of the id register is %x\n",*rb);
	assert(*rb == 0x50);
	printf("Light sensor active\n");

/*

    printf("\nWriting to the interrupt threshold registers\n");
    lightSensorWrite(fd,THRESHLOWLOW,0x03,2); //writing to control register.
    lightSensorWrite(fd,THRESHLOWHIGH,0x04,2); //writing to control register.
    lightSensorWrite(fd,THRESHHIGHLOW,0x05,2); //writing to control register.
    lightSensorWrite(fd,THRESHHIGHHIGH,0x06,2); //writing to control register.

    printf("\n Reading from the interrupt threshold low register\n");
    rb=lightSensorRead(fd,THRESHLOWLOW,2); 
    printf("\n 2byte value read from threshold low low reg is %d\n",rb[0]);
    printf("\n 2byte value read from threshold low high reg is %d\n",rb[1]);

    printf("\n Reading from the interrupt threshold high register\n");
    rb=lightSensorRead(fd,THRESHHIGHLOW,2); 
    printf("\n 2byte value read from threshold high low reg is %d\n",rb[0]);
    printf("\n 2byte value read from threshold high high reg is %d\n",rb[1]); 
	

*/
   
    //printf("\nNow calling lux calc\n");
    //luxCalc(fd);

}

