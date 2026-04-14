obj-m += driver_class_create_read_write.o

KVER ?= $(shell ls -1dt /usr/src/linux-headers-*rpt-rpi-v8 2>/dev/null | head -n1 | sed 's#.*/linux-headers-##')
KDIR ?= /usr/src/linux-headers-$(KVER)
PWD := $(shell pwd)
BUILD_DIR ?= $(PWD)/build
export QEMU_LD_PREFIX ?= /usr/aarch64-linux-gnu

all:
	@test -n "$(KVER)" || (echo "ERROR: no /usr/src/linux-headers-*rpt-rpi-v8 found"; exit 1)
	@test -d "$(KDIR)" || (echo "ERROR: missing headers dir: $(KDIR)"; exit 1)
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) MO=$(BUILD_DIR) clean
	$(RM) -r $(BUILD_DIR)

in-tree:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

.PHONY: all clean in-tree