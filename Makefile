ifneq ($(KERNELRELEASE),)
        obj-m := DCacheDisable.o
else
        KERN_DIR := ~/Desktop/linux-driver/linux-xlnx-xlnx_rebase_v4.14_2018.3
        PWD := $(shell pwd)
default:
	$(MAKE) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KERN_DIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

