#include <stdio.h>
#include <assert.h>

#include "../temp_i2c.h"

int main()
{
	float temp_ferh_comp, temp_kelvin_comp, ferh_store;
	float temperature;
	temperature	= 26.000000;
	temp_ferh_comp = convert_to_unit(temperature, Fahrenheit);
	ferh_store = (temperature * 1.8) + 32;
	assert(temp_ferh_comp == ferh_store);
	printf("Fahrenheit conversion success\n");
	temp_kelvin_comp = convert_to_unit(temperature, Kelvin);
	assert(temp_kelvin_comp == temperature + 273.5);
	printf("Kelvin conversion success\n");
	return 0;
}
