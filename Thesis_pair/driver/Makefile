
obj-m += cryptocore_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	insmod cryptocore_driver.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rmmod cryptocore_driver

