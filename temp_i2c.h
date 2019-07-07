/************************************************************************************************
* File name   : temp_i2c.h                                                                      *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions used for reading and configuring temperature sensor TMP102 for    *
*               getting the temperature values through i2c interface.                           *
* Tools used  : GNU make, gcc, arm-linux-gcc.                                                   *
************************************************************************************************/
#include <stdint.h>

/* Format of meassge that is sent to temperature thread */
// typedef struct{
//     int temperature_unit;
//     char* source;
// }temp_msg;

/* Enumeration for temperature units available */
typedef enum {
Celsius,
Fahrenheit,
Kelvin,
}temperature_unit;

#define DEFAULT_UNIT Celsius;

/* Default slave address */
#define DEFAULT_SLAVE_ADDRESS 0X48

/* The defines for addresses of the registers */
#define TEMP_REG_ADDR   0x00
#define CONFIG_REG_ADDR 0x01
#define TLOW_REG_ADDR   0x02
#define THIGH_REG_ADDR  0x03


/* The macros for configuration register */
#define DEFAULT_CONFIG        0x60A0
#define MODE_12BIT            (0 << 4)
#define MODE_13BIT            (1 << 4)
#define REFRESH_025HZ         (0 << 6)
#define REFRESH_1HZ           (1 << 6)
#define REFRESH_4HZ           (2 << 6)
#define REFRESH_8HZ           (3 << 6)
#define SHUTDOWN_MODE_ENABLE  (1 << 8)
#define SHUTDOWN_MODE_DISABLE (0 << 8)
#define INTERRUPT_MODE        (1 << 9)
#define COMPARATOR_MODE       (0 << 9)
#define ALERT_ACTIVE_HIGH     (1 << 10)
#define ALERT_ACTIVE_LOW      (0 << 10)
#define ALERT_ON_1_FAULT      (0 << 11)
#define ALERT_ON_2_FAULT      (1 << 11)
#define ALERT_ON_4_FAULT      (2 << 11)
#define ALERT_ON_6_FAULT      (3 << 11)
#define START_CONVERSION      (1 << 15)


/* Default values for configuring the registers */
#define TLOW_REG_DEFAULT 25
#define THIGH_REG_DEFAULT 35

/* Macros for queue setup */
//#define TEMP_SENS_QUEUE  "/temp_sens"
//#define TEMP_QUEUE_SIZE  10

/* Macros for timer setup */
#define CLOCK_TO_USE               CLOCK_REALTIME
#define SIGNAL_NOTIFICATION_METHOD SIGEV_SIGNAL
#define SIGNAL_NO                  SIGRTMIN
#define TIME_IN_NANOSEC            100000000  //100msec

/* The functions that are used to communicate to the i2c temperature sensor TMP102 */

/*
* Function name:- temp_i2c_init
* Description:- This function opens the i2c file for i2c transactions. It then sets
*               the slave address for the transactions according to the parameter.
* @param:- uint8_t (slave address)
* @return:- int (file descriptor)
*/
int temp_i2c_init(uint8_t);


/*
* Function name:- temp_i2c_write_to_reg
* Description:- This function takes the file descriptor as parameter which is used to
*               write to a file. It writes the data to the temperature sensor register
*               which is described in parameter. For writing the data to THIGH or TLOW
*               regiser the data to write should be in Celsius.
* @param:- int (file descriptor), uint8_t (temperature sensor register address),
*          int16_t (data to write)
* @return:- int (return error)
*/
int temp_i2c_write_to_reg(int, uint8_t, int16_t);


/*
* Function name:- temp_i2c_read_from_reg
* Description:- This function takes the file descriptor as parameter which is used to
*               write to a file. It reads the received i2c data from register passed
*               and return the buffer value in uint16_t format.
* @param:- int (file descriptor), uint8_t (temperature sensor register address),
* @return:- uint16_t (contents of the register)
*/
uint16_t temp_i2c_read_from_reg(int, uint8_t);


/*
* Function name:- read_temperature
* Description:- This function takes the file descriptor as parameter which is used to
*               write to a file. It formats the data of the register passed in the format
*               of Celcius unit. As configuration register does not contain
*               temperature passing config register address will cause an error.
* @param:- int (file descriptor), uint8_t (temperature sensor register address),
* @return:- float (temperature in Celsius)
*/
float read_temperature(int, uint8_t);


/*
* Function name:- convert_to_unit
* Description:- This function takes temperature and unit to conver to and then converts
*               the value in required temperature unit and returns the value.
* @param:- float (value to be converted), int (temperature unit to convert to),
* @return:- float (temperature in requested unit)
*/
float convert_to_unit(float, int);
