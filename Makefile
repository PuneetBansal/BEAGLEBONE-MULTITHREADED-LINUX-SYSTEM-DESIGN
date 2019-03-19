
prog: main.o gpio.o temp_i2c.o
	arm-linux-gcc main.o gpio.o temp_i2c.o -o main

main.o: main.c gpio.h temp_i2c.h
	arm-linux-gcc -c main.c

gpio.o: gpio.c gpio.h
	arm-linux-gcc -c gpio.c

temp_i2c.o: temp_i2c.c temp_i2c.h my_i2c.h
	arm-linux-gcc -c temp_i2c.c

clean: 
	rm *.o
