obj-m = memguard.o

KVERSION = $(shell uname -r)
BLDDIR= /lib/modules/$(KVERSION)/build

EXTRA_CFLAGS += -g

all: 
	make -C $(BLDDIR) M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	make -C $(BLDDIR) M=$(PWD) clean
