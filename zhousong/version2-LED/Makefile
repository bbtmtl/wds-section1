ifneq ($(KERNELRELEASE),)
obj-m   :=sam_led.o
else
KERN_DIR = /home/sam/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -rf *.core *.order *.symvers *.cmd .tmp_versions \
        *.o *.ko *.mod.c
.PHONY: modules modules_install clean
endif
