###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
# Revision 1.4  2006/11/10 17:51:01  marcus
# Added install and uninstall entries.
#
# Revision 1.3  2006/10/16 16:55:33  marcus
# Added -fPIC to the CFLAGS.
#
# Revision 1.2  2006/10/13 17:18:33  marcus
# Implemented and tested most of C++ interface.
#
# Revision 1.1  2006/10/10 14:46:50  marcus
# Initial commit of the new pciDriver for kernel 2.6
#
###############################################################

# Makefile for the pciDriver library

include ../../common.mk

CFLAGS += -fPIC
CXXFLAGS += -fPIC

LIB ?= $(LIBDIR)/libpcidriver.so
LIBSTATIC ?= $(LIBDIR)/libpcidriver.a

INCDIR += ../../include/lib

LDINC += -L $(LIBDIR)
CXXFLAGS += $(addprefix -I ,$(INCDIR))

###############################################################
# Target definitions

.PHONY: all depend clean

all: depend $(LIB) $(LIBSTATIC)


# Target for dynamic the library
$(LIB): $(OBJ)
	@echo -e "LD [.so] \t$@"
	$(Q)$(CXX) -shared $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

# Target for static the library
$(LIBSTATIC): $(OBJ)
	@echo -e "LD [.a] \t$@"
	$(Q)ar -r $(LIBSTATIC) $^
	$(Q)ar -s $(LIBSTATIC)

install:
	@echo "INSTALL libpcidriver.so"
	-$(Q)install -m 755 $(LIB) /usr/lib/
	@echo "INSTALL libpcidriver.a"
	-$(Q)install -m 755 $(LIBSTATIC) /usr/lib/
	@echo "INSTALL /usr/include/pciDriver/lib"
	-$(Q)mkdir -p /usr/include/pciDriver/lib
	-$(Q)install -D -m 644 ../../include/lib/*.h /usr/include/pciDriver/lib

uninstall:
	@echo "UNINSTALL libpcidriver.so"
	-$(Q)rm -f /usr/lib/libpcidriver.so 
	@echo "UNINSTALL libpcidriver.a"
	-$(Q)rm -f /usr/lib/libpcidriver.a
	@echo "UNINSTALL /usr/include/pciDriver/lib"
	-$(Q)rm -rf /usr/include/pciDriver/lib
	-$(Q)rmdir /usr/include/pciDriver
	        	
clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(LIB) $(LIBSTATIC)
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
