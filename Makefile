
prog: gpio54.o gpio.o 
	arm-linux-gcc gpio54.o gpio.o -o gpio54

gpio54.o: gpio54.c gpio.h
	arm-linux-gcc -c gpio54.c

gpio.o: gpio.c gpio.h
	arm-linux-gcc -c gpio.c

clean: 
	rm *.o
