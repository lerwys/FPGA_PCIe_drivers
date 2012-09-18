###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
# Revision 1.2  2007-05-29 07:50:50  marcus
# Backup commit.
#
# Revision 1.1  2007/02/12 18:09:14  marcus
# Initial commit.
#
###############################################################

# Makefile for the mprace library

include ../../common.mk

CFLAGS += -fPIC
CXXFLAGS += -fPIC

OBJUTIL = Timer.o

LIB ?= $(LIBDIR)/libmprace.so
LIBSTATIC ?= $(LIBDIR)/libmprace.a

INCDIR += ../../include/mprace

LDINC += -L $(LIBDIR)
CXXFLAGS += $(addprefix -I ,$(INCDIR))

###############################################################
# Target definitions

.PHONY: all depend install uninstall clean #util

all: depend $(LIB) $(LIBSTATIC) #util

#util:
#	$(Q)$(MAKE) $(MKOPTS) -C $(SRCDIR)/lib/util all

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
	@echo "INSTALL libmprace.so"
	-$(Q)install -m 755 $(LIB) /usr/lib/
	@echo "INSTALL libmprace.a"
	-$(Q)install -m 755 $(LIBSTATIC) /usr/lib/
	@echo "INSTALL /usr/include/mprace"
	-$(Q)mkdir -p /usr/include/mprace
	-$(Q)install -D -m 644 ../../include/mprace/*.h /usr/include/mprace

uninstall:
	@echo "UNINSTALL libmprace.so"
	-$(Q)rm -f /usr/lib/libmprace.so 
	@echo "UNINSTALL libmprace.a"
	-$(Q)rm -f /usr/lib/libmprace.a
	@echo "UNINSTALL /usr/include/mprace"
	-$(Q)rm -rf /usr/include/mprace
	-$(Q)rmdir /usr/include/mprace

clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(LIB) $(LIBSTATIC)
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
