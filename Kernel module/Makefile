LINUX_PATH:= ~/buildroot/output/build/linux-4796173fc58688055a99a1cef19a839174067220$


obj-m := kernel_module.o

all : 
	make ARCH=arm CROSS_COMPILE=arm-linux- -C $(LINUX_PATH) M=$(shell pwd) modules

clean :
	make -C $(LINUX_PATH) M=$(shell pwd) clean
