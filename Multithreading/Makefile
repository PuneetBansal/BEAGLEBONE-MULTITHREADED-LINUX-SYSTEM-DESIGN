maintask: maintask.o
	gcc maintask.c mq.c logger.c -o maintask -pthread -lrt

maintask_cc: maintask.o
	arm-linux-gcc maintask.c mq.c logger.c -o maintask -pthread -lrt

clean :
	rm *.o *.txt
