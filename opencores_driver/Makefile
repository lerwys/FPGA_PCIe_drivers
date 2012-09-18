###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
# Revision 1.4  2006/11/21 15:54:22  marcus
# Moved board tests to a separate directory, 'other_tests'.
#
# Revision 1.3  2006/11/10 17:51:22  marcus
# Added install and uninstall entries.
#
# Revision 1.2  2006/10/13 17:18:35  marcus
# Implemented and tested most of C++ interface.
#
# Revision 1.1  2006/10/10 14:46:49  marcus
# Initial commit of the new pciDriver for kernel 2.6
#
# Revision 1.1  2006/08/20 13:21:55  marcus
# Initial commit
#
###############################################################

# Main Makefile for the new pciDriver

FLAGS :=

# Use this in the cluster
#FLAGS := -march=nocona -m64

# For opterons
#FLAGS := -march=opteron -m64


# These are switches for the library
# <none>

# These are switches for the driver
# <none>

# These are only for the test programs
# <none>

#export VERBOSE := 1

export CXX := g++
export CC := gcc
export CXXFLAGS += -Wl,--no-as-needed -O2 -g $(FLAGS)
export CFLAGS += -Wl,--no-as-needed -O2 -g $(FLAGS)
export LDFLAGS += -lpthread

ROOTDIR := $(shell pwd)

export INCDIR := $(ROOTDIR)/include
export SRCDIR := $(ROOTDIR)/src
export BINDIR := $(ROOTDIR)/bin
export LIBDIR := $(ROOTDIR)/lib
export OBJDIR := $(ROOTDIR)/obj
export DEPENDDIR := $(ROOTDIR)/depend

ifdef VERBOSE
	Q ?=
	MKOPTS :=
else
    Q ?= @
	MKOPTS := --no-print-directory
endif

###############################################################
# Target definitions

.PHONY: all dirs driver lib tests other_tests install uninstall clean doc

all: dirs driver lib tests

dirs:
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(OBJDIR)
	$(Q)mkdir -p $(LIBDIR)
	$(Q)mkdir -p $(DEPENDDIR)

driver:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/driver

lib:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/lib all

tests:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/tests all

other_tests:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/other_tests all

doc:
	$(Q)doxygen pciDriver.doxygen

install:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/driver install
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/lib install
	@echo "INSTALL 60-udev.fpga.permissions"
	-$(Q)install -m 644 etc/udev/permissions.d/60-udev.fpga.permissions /etc/udev/permissions.d/

uninstall:
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/driver uninstall
	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/lib uninstall
	@echo "UNINSTALL 60-udev.fpga.permissions"
	-$(Q)rm -f /etc/udev/permissions.d/60-udev.fpga.permissions

clean:
	@echo "Cleaning..."
	-$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/driver clean
	-$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/lib clean
	-$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/tests clean
	-$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/other_tests clean

