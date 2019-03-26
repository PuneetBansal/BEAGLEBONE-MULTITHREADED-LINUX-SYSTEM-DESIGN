#include <stdint.h>

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

//char fileName[50];
int myi2cInit(uint8_t slaveAdd);
uint8_t* myi2cRead(int, uint8_t);
int myi2cWrite(int,uint8_t[],uint8_t);


