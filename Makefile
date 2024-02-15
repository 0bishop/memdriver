.SILENT:

obj-m += driver/driver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
TARGET_DIR := target

all: | $(TARGET_DIR)
	rm -rf $(TARGET_DIR)/*
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	# mv *.order *.symvers driver/*.o driver/*.ko driver/*.mod driver/*.mod.c $(TARGET_DIR)
	mv .*.o *.order *.symvers .*.cmd driver/*.o driver/*.ko driver/.*.cmd driver/*.mod driver/*.mod.c $(TARGET_DIR)

	clang++ main.cpp -o usermod -s

test: all
	$(MAKE) -C tester
	printf "\n"
	dmesg | tail -n 2

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf $(TARGET_DIR)
	rm usermod
	@echo "All the shit is cleaned"

cleantest: clean
	$(MAKE) -C tester clean

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: all test clean cleantest $(TARGET_DIR)
