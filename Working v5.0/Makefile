maintask: maintask.o
	gcc maintask.c mq.c logger.c -o maintask -pthread -lrt

maintask_cc: maintask.o
	arm-linux-gcc maintask.c mq.c logger.c temp_i2c.c myi2c.c lightsensor.c gpio.c bist.c -o maintask -pthread -lrt

clean :
	rm *.o *.txt
