obj-m += hello.o

KVER ?= 6.12.47+rpt-rpi-v8
KDIR ?= /usr/src/linux-headers-$(KVER)
PWD  := $(shell pwd)
export QEMU_LD_PREFIX ?= /usr/aarch64-linux-gnu

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
