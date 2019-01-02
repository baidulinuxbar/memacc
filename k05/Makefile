obj-m :=k05.o
pwd :=$(shell pwd)
kernel_dir :=/lib/modules/$(shell uname -r)/build
all:
	make -C $(kernel_dir) M=$(pwd) modules
clean:
	make -C $(kernel_dir) M=$(pwd) clean 

