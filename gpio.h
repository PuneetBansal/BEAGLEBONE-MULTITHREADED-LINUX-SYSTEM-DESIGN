/************************************************************************************************
* File name   : gpio.h                                                                          *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for gpio operations. Setting the direction of pin and        *
*               the value.                                                                      *
* Tools used  : GNU make.                                                                       *
************************************************************************************************/

/**************** Enumerations used for gpio direction and gpio value ****************/
enum gpio_direction{
in = 0,
out,
};

enum gpio_value{
low = 0,
high,
};

/********************** Functions for the gpio operations *********************/
/* 
* Function name:- gpio_init
* Description:- The functon takes the gpio pin number and assignes it as input pin or output pin.
* @param:- int (gpio pin number), int (gpio pin direction)
* @return:- void
* gpio pin direction - 0 for in and 1 for out.
*/
void gpio_init(int,int);


/*
* Function name:- gpio_write_value
* Description:- The functon takes the gpio pin number and outputs the pin high or low.
* @param:- int (gpio pin number), int (gpio pin value)
* @return:- void
* gpio pin direction - 0 for in and 1 for out.
*/
void gpio_write_value(int,int);

