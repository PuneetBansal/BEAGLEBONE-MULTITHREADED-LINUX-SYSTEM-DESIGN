/************************************************************************************************
* File name   : gpio.h                                                                          *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for gpio operations. Setting the direction of pin and        *
*               the value.                                                                      *
* Tools used  : GNU make, gcc, gcc-linux-gcc.                                                   *
************************************************************************************************/
#include <stdbool.h>

#define total_gpio 5
#define access_pin_allowed {53,54,55,56,60}

/**************** Enumerations used for gpio direction and gpio value ****************/
enum gpio_direction{
in = 0,
out,
};

enum gpio_value{
low = 0,
high,
};

typedef enum{
falling,
rising,
both,
none,
}gpio_interrupt;

/********************** Functions for the gpio operations *********************/
/* 
* Function name:- gpio_init
* Description:- The function takes the gpio pin number and assignes it as input pin or 
*               output pin.
* @param:- int (gpio pin number), int (gpio pin direction)
* @return:- void
* gpio pin direction - 0 for in and 1 for out.
*/
void gpio_init(int,int);


/*
* Function name:- gpio_write_value
* Description:- The function takes the gpio pin number and outputs the pin high or low.
* @param:- int (gpio pin number), int (gpio pin value)
* @return:- void
* gpio pin direction - 0 for in and 1 for out.
*/
void gpio_write_value(int,int);


/*
* Function name:- gpio_read_value
* Description:- The function takes the gpio pin number and returns the value on the pin.
* @param:- int (gpio pin number), int (gpio pin value)
* @return:- int (value high or low)
*/
int gpio_read_value(int);


/*
* Function name:- is_pin_valid
* Description:- The function takes the gpio pin number and returns if valid pin no is entered.
* @param:- int (gpio pin number)
* @return:- bool (true if pin number is valid and false if not)
* gpio pin direction - 0 for in and 1 for out.
* Need to maintain pin values and no of valid pins in above define.
*/
bool is_pin_valid(int);


/*
* Function name:- gpio_interrupt_state
* Description:- The function takes the gpio pin number and sets the gpio interrupt as rising
*               falling, both or none based on second parameter.
* @param:- int (gpio pin number), gpio_interrupt (which interrupt);
* @return:- void
* Comments:- gpio_interrupt: can be falling, rising, both or none to disable the interrupts.
*            Need to maintain pin values and no of valid pins in above define.
*/
void gpio_interrupt_state(int, gpio_interrupt);


/*
* Function name:- gpio_open_value
* Description:- The function takes the gpio pin number and opens the file and returns the
*               file descriptor.
* @param:- int (gpio pin number)
* @return:- int (file descriptor)
* Comments:- Need to maintain pin values and no of valid pins in above define.
*/
int gpio_open_value(int);


/*
* Function name:- gpio_read_val_with_fd
* Description:- The function takes the file descriptior of gpio pin and reurnts the state of the
*               pin wether high or low.
* @param:- int (file descriptor)
* @return:- int (pin state)
* Comments:- pin state: Pin state can be high or low.
*/
int gpio_read_val_with_fd(int);
