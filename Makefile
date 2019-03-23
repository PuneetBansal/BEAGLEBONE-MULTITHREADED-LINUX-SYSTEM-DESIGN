
prog: main_i2c_test.o gpio.o temp_i2c.o myi2c.o
	arm-linux-gcc main_i2c_test.o gpio.o temp_i2c.o myi2c.o -o main
	rm *.o

main_i2c_test.o: main_i2c_test.c gpio.h temp_i2c.h
	arm-linux-gcc -c main_i2c_test.c

gpio.o: gpio.c gpio.h
	arm-linux-gcc -c gpio.c

temp_i2c.o: temp_i2c.c temp_i2c.h myi2c.h
	arm-linux-gcc -c temp_i2c.c

myi2c.o: myi2c.c myi2c.h
	arm-linux-gcc -c myi2c.c

clean: 
	rm *.o
