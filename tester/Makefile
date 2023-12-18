.SILENT:

obj-m += tester.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
TARGET_DIR := target

all: | $(TARGET_DIR)
	rm -rf $(TARGET_DIR)/*
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mv *.ko *.mod.c *.order *.symvers *.o *.mod $(TARGET_DIR)
	insmod target/tester.ko

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rmmod tester
	rm -rf $(TARGET_DIR)

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: all clean $(TARGET_DIR)