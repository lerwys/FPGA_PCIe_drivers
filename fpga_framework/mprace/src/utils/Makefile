###############################################################
#
# Change History:
#
# $Log: not supported by cvs2svn $
###############################################################

# Makefile for the mprace library utilities

include ../../common.mk

INCDIR += ../../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -lpcidriver -lmprace

BINARIES = setclock fpgaconfig adcread

###############################################################
# Target definitions

.PHONY: all dirs depend clean

all: dirs depend $(BINARIES)

# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $@ $<

install:
	@echo "INSTALL /usr/include/mprace/util"
	-$(Q)mkdir -p /usr/include/mprace/util
	-$(Q)install -m 644 ../../include/mprace/util/*.h /usr/include/mprace/util

#$(BINDIR)/testPciDriver: $(OBJDIR)/testPciDriver.o
#	@echo -e "LD \t$@"
#	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $<
	    	
clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
