#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include "gpio.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


static void handler(int sig, siginfo_t *si, void *data)
{
	printf("Signal received %d",sig);	
}

int main()
{
	FILE *fp;
	char *file = (char*)malloc(40);
	struct sigaction signal_act;
	int fd, wd;

	gpio_init(60,in);
	printf("gpio_init done\n");
	file = "/sys/class/gpio/gpio60";
	printf("file is %s\n",file);	

	while(1);
	return 0;
}
