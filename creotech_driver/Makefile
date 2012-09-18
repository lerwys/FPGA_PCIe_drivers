

obj-m		+= pcie.o
#obj-m		+= vivi.o

# Change this to reflect the location of the Linux kernel sources
KDIR            := /home/jacek/buildroot.p2020/output/build/linux-3.1.6
ARCH		?= powerpc
CROSS_COMPILE	?= powerpc-linux-
PATH		:=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/home/jacek/buildroot.p2020/output/host/usr/bin
export ARCH CROSS_COMPILE
export PATH
EXTRA_CFLAGS	:= -D VERBOSE

KBUILD_TARGETS	:= clean help modules modules_install

.PHONY: all $(KBUILD_TARGETS)
all: modules

$(KBUILD_TARGETS):
	$(MAKE) -C $(KDIR) M=$(shell pwd) $@
	cp pcie.ko /opt/nfsroot/root
#	cp pcie.ko /home/jacek/targetfsp2020

