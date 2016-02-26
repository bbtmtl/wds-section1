# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m :=hellomodule.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?=  /root/linux-2.6.22.6
	PWD :=$(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf modules.order  *.o *.mod *.mod.c *.symvers *.ko
endif
