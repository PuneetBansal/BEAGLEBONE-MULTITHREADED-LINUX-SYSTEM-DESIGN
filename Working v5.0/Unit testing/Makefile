
temp:
	arm-linux-gcc main_i2c_test.c ../temp_i2c.c ../myi2c.c -o temp_test
	
conv:
	arm-linux-gcc temp_conv_test.c ../temp_i2c.c ../myi2c.c -o conv_test

light:
	arm-linux-gcc main.c ../lightsensor.c ../myi2c.c -o light_test
